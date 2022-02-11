#include "numc.h"
#include <structmember.h>

PyTypeObject Matrix61cType;

/* Helper functions for initalization of matrices and vectors */

/*
 * Return a tuple given rows and cols
 */
PyObject *get_shape(int rows, int cols) {
  if (rows == 1 || cols == 1) {
    return PyTuple_Pack(1, PyLong_FromLong(rows * cols));
  } else {
    return PyTuple_Pack(2, PyLong_FromLong(rows), PyLong_FromLong(cols));
  }
}
/*
 * Matrix(rows, cols, low, high). Fill a matrix random double values
 */
int init_rand(PyObject *self, int rows, int cols, unsigned int seed, double low,
              double high) {
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    rand_matrix(new_mat, seed, low, high);
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * Matrix(rows, cols, val). Fill a matrix of dimension rows * cols with val
 */
int init_fill(PyObject *self, int rows, int cols, double val) {
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed)
        return alloc_failed;
    else {
        fill_matrix(new_mat, val);
        ((Matrix61c *)self)->mat = new_mat;
        ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    }
    return 0;
}

/*
 * Matrix(rows, cols, 1d_list). Fill a matrix with dimension rows * cols with 1d_list values
 */
int init_1d(PyObject *self, int rows, int cols, PyObject *lst) {
    if (rows * cols != PyList_Size(lst)) {
        PyErr_SetString(PyExc_ValueError, "Incorrect number of elements in list");
        return -1;
    }
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    int count = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            set(new_mat, i, j, PyFloat_AsDouble(PyList_GetItem(lst, count)));
            count++;
        }
    }
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * Matrix(2d_list). Fill a matrix with dimension len(2d_list) * len(2d_list[0])
 */
int init_2d(PyObject *self, PyObject *lst) {
    int rows = PyList_Size(lst);
    if (rows == 0) {
        PyErr_SetString(PyExc_ValueError,
                        "Cannot initialize numc.Matrix with an empty list");
        return -1;
    }
    int cols;
    if (!PyList_Check(PyList_GetItem(lst, 0))) {
        PyErr_SetString(PyExc_ValueError, "List values not valid");
        return -1;
    } else {
        cols = PyList_Size(PyList_GetItem(lst, 0));
    }
    for (int i = 0; i < rows; i++) {
        if (!PyList_Check(PyList_GetItem(lst, i)) ||
                PyList_Size(PyList_GetItem(lst, i)) != cols) {
            PyErr_SetString(PyExc_ValueError, "List values not valid");
            return -1;
        }
    }
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            set(new_mat, i, j,
                PyFloat_AsDouble(PyList_GetItem(PyList_GetItem(lst, i), j)));
        }
    }
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * This deallocation function is called when reference count is 0
 */
void Matrix61c_dealloc(Matrix61c *self) {
    deallocate_matrix(self->mat);
    Py_TYPE(self)->tp_free(self);
}

/* For immutable types all initializations should take place in tp_new */
PyObject *Matrix61c_new(PyTypeObject *type, PyObject *args,
                        PyObject *kwds) {
    /* size of allocated memory is tp_basicsize + nitems*tp_itemsize*/
    Matrix61c *self = (Matrix61c *)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

/*
 * This matrix61c type is mutable, so needs init function. Return 0 on success otherwise -1
 */
int Matrix61c_init(PyObject *self, PyObject *args, PyObject *kwds) {
    /* Generate random matrices */
    if (kwds != NULL) {
        PyObject *rand = PyDict_GetItemString(kwds, "rand");
        if (!rand) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
        if (!PyBool_Check(rand)) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
        if (rand != Py_True) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }

        PyObject *low = PyDict_GetItemString(kwds, "low");
        PyObject *high = PyDict_GetItemString(kwds, "high");
        PyObject *seed = PyDict_GetItemString(kwds, "seed");
        double double_low = 0;
        double double_high = 1;
        unsigned int unsigned_seed = 0;

        if (low) {
            if (PyFloat_Check(low)) {
                double_low = PyFloat_AsDouble(low);
            } else if (PyLong_Check(low)) {
                double_low = PyLong_AsLong(low);
            }
        }

        if (high) {
            if (PyFloat_Check(high)) {
                double_high = PyFloat_AsDouble(high);
            } else if (PyLong_Check(high)) {
                double_high = PyLong_AsLong(high);
            }
        }

        if (double_low >= double_high) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }

        // Set seed if argument exists
        if (seed) {
            if (PyLong_Check(seed)) {
                unsigned_seed = PyLong_AsUnsignedLong(seed);
            }
        }

        PyObject *rows = NULL;
        PyObject *cols = NULL;
        if (PyArg_UnpackTuple(args, "args", 2, 2, &rows, &cols)) {
            if (rows && cols && PyLong_Check(rows) && PyLong_Check(cols)) {
                return init_rand(self, PyLong_AsLong(rows), PyLong_AsLong(cols), unsigned_seed, double_low,
                                 double_high);
            }
        } else {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
    }
    PyObject *arg1 = NULL;
    PyObject *arg2 = NULL;
    PyObject *arg3 = NULL;
    if (PyArg_UnpackTuple(args, "args", 1, 3, &arg1, &arg2, &arg3)) {
        /* arguments are (rows, cols, val) */
        if (arg1 && arg2 && arg3 && PyLong_Check(arg1) && PyLong_Check(arg2) && (PyLong_Check(arg3)
                || PyFloat_Check(arg3))) {
            if (PyLong_Check(arg3)) {
                return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), PyLong_AsLong(arg3));
            } else
                return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), PyFloat_AsDouble(arg3));
        } else if (arg1 && arg2 && arg3 && PyLong_Check(arg1) && PyLong_Check(arg2) && PyList_Check(arg3)) {
            /* Matrix(rows, cols, 1D list) */
            return init_1d(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), arg3);
        } else if (arg1 && PyList_Check(arg1) && arg2 == NULL && arg3 == NULL) {
            /* Matrix(rows, cols, 1D list) */
            return init_2d(self, arg1);
        } else if (arg1 && arg2 && PyLong_Check(arg1) && PyLong_Check(arg2) && arg3 == NULL) {
            /* Matrix(rows, cols, 1D list) */
            return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), 0);
        } else {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return -1;
    }
}

/*
 * List of lists representations for matrices
 */
PyObject *Matrix61c_to_list(Matrix61c *self) {
    int rows = self->mat->rows;
    int cols = self->mat->cols;
    PyObject *py_lst = NULL;
    if (self->mat->is_1d) {  // If 1D matrix, print as a single list
        py_lst = PyList_New(rows * cols);
        int count = 0;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                PyList_SetItem(py_lst, count, PyFloat_FromDouble(get(self->mat, i, j)));
                count++;
            }
        }
    } else {  // if 2D, print as nested list
        py_lst = PyList_New(rows);
        for (int i = 0; i < rows; i++) {
            PyList_SetItem(py_lst, i, PyList_New(cols));
            PyObject *curr_row = PyList_GetItem(py_lst, i);
            for (int j = 0; j < cols; j++) {
                PyList_SetItem(curr_row, j, PyFloat_FromDouble(get(self->mat, i, j)));
            }
        }
    }
    return py_lst;
}

PyObject *Matrix61c_class_to_list(Matrix61c *self, PyObject *args) {
    PyObject *mat = NULL;
    if (PyArg_UnpackTuple(args, "args", 1, 1, &mat)) {
        if (!PyObject_TypeCheck(mat, &Matrix61cType)) {
            PyErr_SetString(PyExc_TypeError, "Argument must of type numc.Matrix!");
            return NULL;
        }
        Matrix61c* mat61c = (Matrix61c*)mat;
        return Matrix61c_to_list(mat61c);
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
    }
}

/*
 * Add class methods
 */
PyMethodDef Matrix61c_class_methods[] = {
    {"to_list", (PyCFunction)Matrix61c_class_to_list, METH_VARARGS, "Returns a list representation of numc.Matrix"},
    {NULL, NULL, 0, NULL}
};

/*
 * Matrix61c string representation. For printing purposes.
 */
PyObject *Matrix61c_repr(PyObject *self) {
    PyObject *py_lst = Matrix61c_to_list((Matrix61c *)self);
    return PyObject_Repr(py_lst);
}

/* NUMBER METHODS */

/*
 * Add the second numc.Matrix (Matrix61c) object to the first one. The first operand is
 * self, and the second operand can be obtained by casting `args`.
 */
PyObject *Matrix61c_add(Matrix61c* self, PyObject* args) {
    /* TODO: YOUR CODE HERE */

    if (PyObject_TypeCheck(args, &Matrix61cType))
    {
      matrix* operand1 = ((Matrix61c*)args)->mat; 
      matrix* res;
      if (allocate_matrix(&res, operand1->rows, operand1->cols) !=0)
      {
        return NULL ;
      }

      if (add_matrix(res, self->mat, operand1)!=0)
      {
        return NULL;
      }

      Matrix61c* res_mat = (Matrix61c*)Matrix61c_new(&Matrix61cType, NULL, NULL);
      res_mat->mat = res; 
      
      return res_mat;
    }
    else
    {
      return NULL;
    }
}

/*
 * Substract the second numc.Matrix (Matrix61c) object from the first one. The first operand is
 * self, and the second operand can be obtained by casting `args`.
 */
PyObject *Matrix61c_sub(Matrix61c* self, PyObject* args) {
    /* TODO: YOUR CODE HERE */
    if (PyObject_TypeCheck(args, &Matrix61cType))
    {
      matrix* operand1 = ((Matrix61c*)args)->mat; 
      matrix* res;
      if (allocate_matrix(&res, operand1->rows, operand1->cols) !=0)
      {
        return NULL ;
      }

      if (sub_matrix(res, self->mat, operand1)!=0)
      {
        return NULL;
      }

      Matrix61c* res_mat = (Matrix61c*)Matrix61c_new(&Matrix61cType, NULL, NULL);
      res_mat->mat = res; 
      
      return res_mat;
    }
    else
    {
      return NULL;
    }
}

/*
 * NOT element-wise multiplication. The first operand is self, and the second operand
 * can be obtained by casting `args`.
 */
PyObject *Matrix61c_multiply(Matrix61c* self, PyObject *args) {
    /* TODO: YOUR CODE HERE */
    if (PyObject_TypeCheck(args, &Matrix61cType))
    {
      matrix* operand1 = ((Matrix61c*)args)->mat; 
      matrix* res;
      if (allocate_matrix(&res, self->mat->rows, operand1->cols) !=0)
      {
        return NULL ;
      }

      if (mul_matrix(res, self->mat, operand1)!=0)
      {
        return NULL;
      }

      Matrix61c* res_mat = (Matrix61c*)Matrix61c_new(&Matrix61cType, NULL, NULL);
      res_mat->mat = res; 
      
      return res_mat;
    }
    else
    {
      return NULL;
    }
}

/*
 * Negates the given numc.Matrix.
 */
PyObject *Matrix61c_neg(Matrix61c* self) {
    /* TODO: YOUR CODE HERE */
      matrix* res;
      if (allocate_matrix(&res, self->mat->rows, self->mat->cols) !=0)
      {
        return NULL ;
      }


      if (neg_matrix(res, self->mat) != 0)
      {
        return NULL; 
      }

      Matrix61c* res_mat = (Matrix61c*)Matrix61c_new(&Matrix61cType, NULL, NULL);
      res_mat->mat = res; 
}

/*
 * Take the element-wise absolute value of this numc.Matrix.
 */
PyObject *Matrix61c_abs(Matrix61c *self) {
    /* TODO: YOUR CODE HERE */
      matrix* res;
      if (allocate_matrix(&res, self->mat->rows, self->mat->cols) !=0)
      {
        return NULL ;
      }


      if (abs_matrix(res, self->mat) != 0)
      {
        return NULL; 
      }

      Matrix61c* res_mat = (Matrix61c*)Matrix61c_new(&Matrix61cType, NULL, NULL);
      res_mat->mat = res; 
    
}

/*
 * Raise numc.Matrix (Matrix61c) to the `pow`th power. You can ignore the argument `optional`.
 */
PyObject *Matrix61c_pow(Matrix61c *self, PyObject *pow, PyObject *optional) {
    /* TODO: YOUR CODE HERE */
  if(PyLong_Check(pow))
  {
      matrix* res;
      if (allocate_matrix(&res, self->mat->rows, self->mat->cols) !=0)
      {
        return NULL ;
      }

      if (pow_matrix(res, self->mat, PyLong_AsLong(pow))!=0)
      {
        return NULL;
      }

      Matrix61c* res_mat = (Matrix61c*)Matrix61c_new(&Matrix61cType, NULL, NULL);
      res_mat->mat = res; 
      
      return res_mat;
  }
  else
  {
    return NULL;
  }
}

/*
 * Create a PyNumberMethods struct for overloading operators with all the number methods you have
 * define. You might find this link helpful: https://docs.python.org/3.6/c-api/typeobj.html
 */
PyNumberMethods Matrix61c_as_number = {
    /* TODO: YOUR CODE HERE */
    (binaryfunc) Matrix61c_add,
    (binaryfunc) Matrix61c_sub,
    (binaryfunc) Matrix61c_multiply,
    NULL, //remainder
    NULL, //divmod
    (ternaryfunc) Matrix61c_pow,
    (unaryfunc) Matrix61c_neg,
    NULL, //positive
    (unaryfunc)Matrix61c_abs

};


/* INSTANCE METHODS */

/*
 * Given a numc.Matrix self, parse `args` to (int) row, (int) col, and (double/int) val.
 * Return None in Python (this is different from returning null).
 */
PyObject *Matrix61c_set_value(Matrix61c *self, PyObject* args) {
    /* TODO: YOUR CODE HERE */
    PyObject *col = NULL;
    PyObject *row = NULL;
    PyObject *val = NULL;
    if (PyArg_UnpackTuple(args, "args", 3, 3, &row, &col, &val))
    {
        set(self->mat, PyLong_AsLong(row), PyLong_AsLong(col), PyFloat_AsDouble(val));
        //return Py_None;
        Py_RETURN_NONE;
       
    }
    else
    {
      PyErr_SetString(PyExc_TypeError, "Failed to unpack args when calling  set function!");
      return NULL;
    }
}

/*
 * Given a numc.Matrix `self`, parse `args` to (int) row and (int) col.
 * Return the value at the `row`th row and `col`th column, which is a Python
 * float/int.
 */
PyObject *Matrix61c_get_value(Matrix61c *self, PyObject* args) {
    /* TODO: YOUR CODE HERE */
    PyObject *col = NULL;
    PyObject *row = NULL;
    if (PyArg_UnpackTuple(args, "args", 2, 2, &row, &col))
    {
      return PyFloat_FromDouble(get(self->mat, PyLong_AsLong(row), PyLong_AsLong(col))); 
    }
    else
    {
      return NULL;
    }
}

/*
 * Create an array of PyMethodDef structs to hold the instance methods.
 * Name the python function corresponding to Matrix61c_get_value as "get" and Matrix61c_set_value
 * as "set"
 * You might find this link helpful: https://docs.python.org/3.6/c-api/structures.html
 */
PyMethodDef Matrix61c_methods[] = {
    /* TODO: YOUR CODE HERE */
    {"get", (PyCFunction)Matrix61c_get_value, METH_VARARGS, "Get an element's value from a give position."},
    {"set", Matrix61c_set_value, METH_VARARGS, "Set an element's value from a give position."},
    {NULL, NULL, 0, NULL}
};

/* INDEXING */

/*
 * Given a numc.Matrix `self`, index into it with `key`. Return the indexed result.
 */
PyObject *Matrix61c_subscript(Matrix61c* self, PyObject* key) {
    /* TODO: YOUR CODE HERE */


    //printf("HELLO tuple %d List %d long %d  slice %d\n", PyTuple_Check(key), PyList_Check(key), PyLong_Check(key), PySlice_Check(key));

    if (key == NULL)
    {
      return NULL;
    }
    else if (PyLong_Check(key))
    {
      PyObject* py_lst;
      py_lst = PyList_New(self->mat->cols);
      for(int i = 0; i< self->mat->cols; i++)
      {
        PyList_SetItem(py_lst, i, PyFloat_FromDouble(get(self->mat, PyLong_AsLong(key), i)));
      }
      return py_lst;
    }
    else if (PySlice_Check(key))
    {
      Py_ssize_t start;
      Py_ssize_t step ;
      Py_ssize_t end ;
      Py_ssize_t slice_length ;
      if (PySlice_GetIndicesEx(key, self->mat->rows, &start, &end, &step, &slice_length) == 0)
      {
        PyObject * py_lst;

        py_lst = PyList_New(slice_length);

        int row = 0;
        for (int i = start ; i< end; i+=step)
        {

          PyList_SetItem(py_lst, row, PyList_New(self->mat->cols));
          PyObject *curr_row = PyList_GetItem(py_lst, row);
          row ++;
          for(int j = 0; j< self->mat->cols; j++)
          {
            PyList_SetItem(curr_row, j, PyFloat_FromDouble(get(self->mat, i, j)));
          }
        }
        return py_lst;
      }
      else
      {
        PyErr_SetString(PyExc_TypeError, "Invalid slice format!");
        return NULL;
      }
    }
    else if (PyTuple_Check(key))
    {
      PyObject *arg1 = NULL;
      PyObject *arg2 = NULL;
      if (PyArg_UnpackTuple(key, "args", 2, 2, &arg1, &arg2))
      {
        PyObject * py_lst;
        Py_ssize_t start1;
        Py_ssize_t step1 ;
        Py_ssize_t end1 ;
        Py_ssize_t slice_length1 ;
        Py_ssize_t start2;
        Py_ssize_t step2 ;
        Py_ssize_t end2 ;
        Py_ssize_t slice_length2 ;

        if (PySlice_Check(arg1))
        {
            if (!PySlice_GetIndicesEx(arg1, self->mat->rows, &start1, &end1, &step1, &slice_length1) == 0)
            {
              PyErr_SetString(PyExc_TypeError, "Invalid slice format for arg1!");
              return NULL;
            }
        }
        else if (PyLong_Check(arg1))
        {
          slice_length1 = -1;
          start1 = PyLong_AsLong(arg1);
        }
        else
        {
          PyErr_SetString(PyExc_TypeError, "Invalid first args in tuple!");
          return NULL;
        }

        if (PySlice_Check(arg2))
        {
            if (!PySlice_GetIndicesEx(arg2, self->mat->rows, &start2, &end2, &step2, &slice_length2) == 0)
            {
              PyErr_SetString(PyExc_TypeError, "Invalid slice format for arg2!");
              return NULL;
            }
        }
        else if (PyLong_Check(arg2))
        {
          slice_length2 = -1;
          start2 = PyLong_AsLong(arg2);
        }
        else
        {
          PyErr_SetString(PyExc_TypeError, "Invalid second args in tuple!");
          return NULL;
        }

        if (slice_length1 == -1 && slice_length2 == -1)
        {
          return PyFloat_FromDouble(get(self->mat, start1, start2));
        }
        else if (slice_length1 == -1 && slice_length2 != -1)
        {
          py_lst = PyList_New(slice_length2);
          int col = 0;
          for (int i =start2; i< end2; i+=step2)
          {
            PyList_SetItem(py_lst, col, PyFloat_FromDouble(get(self->mat, start1, i)));
            col ++;
          }
          return py_lst;
        }
        else if (slice_length1 != -1 && slice_length2 == -1)
        {
          py_lst = PyList_New(slice_length1);
          int row = 0;
          for (int i =start1; i< end1; i+=step1)
          {
            PyList_SetItem(py_lst, row, PyFloat_FromDouble(get(self->mat, i, start2)));
            row ++;
          }
          return py_lst;
        }
        else if (slice_length1 != -1 && slice_length2 != -1)
        {


          py_lst = PyList_New(slice_length1);

          int row = 0;
          for (int r = start1; r<end1; r+=step1)
          {
              PyList_SetItem(py_lst, row, PyList_New(slice_length2));
              PyObject *curr_row = PyList_GetItem(py_lst, row);
              int col = 0;
              for (int c = start2 ; c< end2; c+=step2)
              {
                PyList_SetItem(curr_row, col, PyFloat_FromDouble(get(self->mat, r, c)));
                col ++;
              }
              row ++;
          }

          return py_lst;


        }
        else
        {
          PyErr_SetString(PyExc_TypeError, "Something is wrong!");
          return NULL;
        }
      }
      else
      {
          PyErr_SetString(PyExc_TypeError, "Invalid tuple format!");
          return NULL;
      }
    }

}

/*
 * Given a numc.Matrix `self`, index into it with `key`, and set the indexed result to `v`.
 */
int Matrix61c_set_subscript(Matrix61c* self, PyObject *key, PyObject *v) {
    /* TODO: YOUR CODE HERE */
}

PyMappingMethods Matrix61c_mapping = {
    NULL,
    (binaryfunc) Matrix61c_subscript,
    (objobjargproc) Matrix61c_set_subscript,
};

/* INSTANCE ATTRIBUTES*/
PyMemberDef Matrix61c_members[] = {
    {
        "shape", T_OBJECT_EX, offsetof(Matrix61c, shape), 0,
        "(rows, cols)"
    },
    {NULL}  /* Sentinel */
};

PyTypeObject Matrix61cType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "numc.Matrix",
    .tp_basicsize = sizeof(Matrix61c),
    .tp_dealloc = (destructor)Matrix61c_dealloc,
    .tp_repr = (reprfunc)Matrix61c_repr,
    .tp_as_number = &Matrix61c_as_number,
    .tp_flags = Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,
    .tp_doc = "numc.Matrix objects",
    .tp_methods = Matrix61c_methods,
    .tp_members = Matrix61c_members,
    .tp_as_mapping = &Matrix61c_mapping,
    .tp_init = (initproc)Matrix61c_init,
    .tp_new = Matrix61c_new
};


struct PyModuleDef numcmodule = {
    PyModuleDef_HEAD_INIT,
    "numc",
    "Numc matrix operations",
    -1,
    Matrix61c_class_methods
};

/* Initialize the numc module */
PyMODINIT_FUNC PyInit_numc(void) {
    PyObject* m;

    if (PyType_Ready(&Matrix61cType) < 0)
        return NULL;

    m = PyModule_Create(&numcmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&Matrix61cType);
    PyModule_AddObject(m, "Matrix", (PyObject *)&Matrix61cType);
    printf("CS61C Fall 2020 Project 4: numc imported!\n");
    fflush(stdout);
    return m;
}
