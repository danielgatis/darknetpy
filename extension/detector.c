#include <Python.h>
#include "darknet.h"
#include "darknet/src/list.h"

typedef struct {
    char cwd[1024];
    char **names;
    network net;
} network_context_type;

typedef struct {
    char name[32];
    int left;
    int right;
    int top;
    int bottom;
    float prob;
} detection_type;

list *
get_detections(image im, int num, float thresh, box *boxes, float **probs, int classes, char **names)
{
    list *detection_list = make_list();

    for(int i = 0; i < num; ++i){
        int class = max_index(probs[i], classes);
        float prob = probs[i][class];

        if(prob > thresh){
            box b = boxes[i];

            int left  = (b.x - b.w / 2.) * im.w;
            int right = (b.x + b.w / 2.) * im.w;
            int top   = (b.y - b.h / 2.) * im.h;
            int bot   = (b.y + b.h / 2.) * im.h;

            if(left < 0) left = 0;
            if(right > im.w - 1) right = im.w - 1;
            if(top < 0) top = 0;
            if(bot > im.h - 1) bot = im.h - 1;

            detection_type *detection = (detection_type *)malloc(sizeof(detection_type));
            strncpy(detection->name, names[class], sizeof(detection->name));
            detection->left = left;
            detection->right = right;
            detection->top = top;
            detection->bottom = bot;
            detection->prob = prob;
            list_insert(detection_list, detection);
        }
    }

    return detection_list;
}

typedef struct {
    PyObject_HEAD
    network_context_type *network_context;
} Detector;

PyObject *
Detector_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Detector *self;
    self = (Detector *)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

void
Detector_dealloc(Detector* self)
{
    if (self->network_context) free(self->network_context);
}

int
Detector_init(Detector *self, PyObject *args, PyObject *kwds)
{
    char *cwd;
    char *datacfg;
    char *cfgfile;
    char *weightfile;

    if (!PyArg_ParseTuple(args, "ssss", &cwd, &datacfg, &cfgfile, &weightfile))
        return -1;

    srand(2222222);

    char currentcwd[1024];
    getcwd(currentcwd, sizeof(currentcwd));
    chdir(cwd);

    network_context_type *network_context = (network_context_type *)malloc(sizeof(network_context_type));
    if (!network_context) return -1;
    memset(network_context, 0, sizeof(network_context_type));

    strncpy(network_context->cwd, cwd, sizeof(network_context->cwd));

    list *options = read_data_cfg(datacfg);
    char *name_list = option_find_str(options, "names", "data/names.list");
    network_context->names = get_labels(name_list);

    network_context->net = parse_network_cfg(cfgfile);
    load_weights(&network_context->net, weightfile);
    set_batch_network(&network_context->net, 1);

    self->network_context = (network_context_type*)network_context;
    chdir(currentcwd);
    return 0;
}

PyObject *
Detector_detect(Detector* self, PyObject *args) {
    char *input;
    float thresh=.24;
    float hier_thresh=.5;
    float nms = .3;

    if (!PyArg_ParseTuple(args, "s|fff", &input, &thresh, &hier_thresh, &nms))
        return NULL;

    network_context_type *obj = (network_context_type *)self->network_context;

    srand(2222222);

    char currentcwd[1024];
    getcwd(currentcwd, sizeof(currentcwd));
    chdir(obj->cwd);

    image im = load_image_color(input, 0, 0);
    image sized = letterbox_image(im, obj->net.w, obj->net.h);
    layer l = obj->net.layers[obj->net.n - 1];

    box *boxes = calloc(l.w * l.h * l.n, sizeof(box));
    float **probs = calloc(l.w * l.h * l.n, sizeof(float *));
    for(int j = 0; j < l.w * l.h * l.n; ++j) probs[j] = calloc(l.classes, sizeof(float *));

    float **masks = 0;
    if (l.coords > 4){
        masks = calloc(l.w * l.h * l.n, sizeof(float*));
        for(int j = 0; j < l.w * l.h * l.n; ++j) masks[j] = calloc(l.coords - 4, sizeof(float *));
    }

    float *X = sized.data;
    network_predict(obj->net, X);
    get_region_boxes(l, im.w, im.h, obj->net.w, obj->net.h, thresh, probs, boxes, masks, 0, 0, hier_thresh, 1);
    if (nms) do_nms_sort(boxes, probs, l.w * l.h * l.n, l.classes, nms);

    list *detection_list = get_detections(im, l.w * l.h * l.n, thresh, boxes, probs, l.classes, obj->names);
    detection_type **detections = (detection_type **)list_to_array(detection_list);
    int num = detection_list->size;

    PyObject *dict = NULL;
    PyObject *list = PyList_New(num);

    if (detections == NULL) {
        return list;
    }

    for (int i = 0; i < num; i++) {
        dict = Py_BuildValue("{s:s,s:i,s:i,s:i,s:i,s:f}",
            "class", detections[i]->name,
            "left", detections[i]->left,
            "right", detections[i]->right,
            "top", detections[i]->top,
            "bottom", detections[i]->bottom,
            "prob", detections[i]->prob);
        PyList_SetItem(list, i, dict);
        free(detections[i]);
    }

    free(detections);
    free_image(im);
    free_image(sized);
    free(boxes);
    free_ptrs((void **)probs, l.w * l.h * l.n);

    chdir(currentcwd);
    return list;
}

PyMethodDef
Detector_methods[] = {
    {
        "detect",
        (PyCFunction)Detector_detect,
        METH_VARARGS,
        NULL
    },
    {
        NULL
    }
};

PyTypeObject
DetectorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "detector.Detector",            /* tp_name */
    sizeof(Detector),               /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)Detector_dealloc,   /* tp_dealloc */
    0,                              /* tp_pri    nt */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_reserved */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash  */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,        /* tp_flags */
    0,                              /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
    Detector_methods,               /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)Detector_init,        /* tp_init */
    0,                              /* tp_alloc */
    Detector_new,                   /* tp_new */
};

PyModuleDef
detector_mod = {
    PyModuleDef_HEAD_INIT,
    "detector",
    NULL,
    -1,
    NULL, NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC
PyInit_detector(void) {
    PyObject* m;

    DetectorType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&DetectorType) < 0)
        return NULL;

    m = PyModule_Create(&detector_mod);
    if (m == NULL)
        return NULL;

    Py_INCREF(&DetectorType);
    PyModule_AddObject(m, "Detector", (PyObject *)&DetectorType);
    return m;
}
