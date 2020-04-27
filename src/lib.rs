#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(dead_code)]
#![allow(unused_attributes)]
#![allow(unused_must_use)]
#![allow(stable_features)]

#![feature(specialization)]
#![feature(proc_macro)]

extern crate pyo3;

use pyo3::prelude::*;
use pyo3::types::{PyDict, PyList};

use std::ffi::{CString, CStr};

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

#[pyclass]
struct Detector {
    network: *mut network,
    metadata: metadata,
}

#[pymethods]
impl Detector {
    #[new]
    fn __new__(meta: String, config: String, weights: String) -> Self {
        let metadata = unsafe { get_metadata(CString::new(meta).expect("invalid meta").into_raw()) };
        let network = unsafe {
            load_network(
                CString::new(config).expect("invalid config").into_raw(),
                CString::new(weights).expect("invalid weights").into_raw(),
                0,
            )
        };

        Detector { network, metadata }
    }

    fn detect(
        &self,
        img_path: String,
        thresh: Option<f32>,
        hier_thresh: Option<f32>,
        nms: Option<f32>,
    ) -> PyObject {
        let thresh = thresh.unwrap_or(0.50f32);
        let hier_thresh = hier_thresh.unwrap_or(0.50f32);
        let nms = nms.unwrap_or(0.50f32);

        let gil = Python::acquire_gil();
        let py = gil.python();

        unsafe {
            set_batch_network(self.network, 1);
            srand(2222222);
        }

        let image = unsafe { load_image_color(CString::new(img_path).expect("invalid img_path").into_raw(), 0, 0) };
        let sized = unsafe { letterbox_image(image, (*self.network).w, (*self.network).h) };

        unsafe { network_predict(self.network, sized.data) };

        let num_ptr = &mut 0 as *mut i32;
        let boxes = unsafe {
            get_network_boxes(
                self.network,
                image.w,
                image.h,
                thresh,
                hier_thresh,
                0 as *mut i32,
                0,
                num_ptr,
            )
        };

        let num = unsafe { *num_ptr };

        if nms > 0. {
            unsafe { do_nms_obj(boxes, num, self.metadata.classes, nms) };
        }

        let list = PyList::empty(py);

        for n in 0..num {
            for c in 0..self.metadata.classes {
                let nbox = unsafe { *boxes.offset(n as isize) };
                let prob = unsafe { *nbox.prob.offset(c as isize) };

                if prob > 0. {
                    let b = nbox.bbox;
                    let class = unsafe {
                        CStr::from_ptr(*self.metadata.names.offset(c as isize)).to_string_lossy().into_owned()
                    };

                    let iw = image.w as f32;
                    let ih = image.h as f32;

                    let mut left = b.x - b.w / 2.;
                    let mut top = b.y - b.h / 2.;
                    let mut right = b.x + b.w / 2.;
                    let mut bottom = b.y + b.h / 2.;

                    if left < 0. {
                        left = 0.;
                    }

                    if top < 0. {
                        top = 0.;
                    }

                    if right > iw {
                        right = iw;
                    }

                    if bottom > ih {
                        bottom = ih;
                    }

                    let item = PyDict::new(py);

                    item.set_item("class", class);
                    item.set_item("prob", prob);
                    item.set_item("left", left);
                    item.set_item("top", top);
                    item.set_item("right", right);
                    item.set_item("bottom", bottom);

                    list.append(item);
                }
            }
        }

        unsafe { free_detections(boxes, num) };
        list.to_object(py)
    }
}

#[pymodule]
fn darknetpy(_py: Python, m: &PyModule) -> PyResult<()> {
    m.add_class::<Detector>()?;
    Ok(())
}
