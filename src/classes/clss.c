/** @file clss.c
 *  @author T J Atherton
 *
 *  @brief Defines class object type
 */

#include "morpho.h"
#include "classes.h"

/* **********************************************************************
 * objectclass definitions
 * ********************************************************************** */

/** Class object definitions */
void objectclass_printfn(object *obj, void *v) {
    morpho_printf(v, "@%s", MORPHO_GETCSTRING(((objectclass *) obj)->name));
}

void objectclass_markfn(object *obj, void *v) {
    objectclass *c = (objectclass *) obj;
    morpho_markvalue(v, c->name);
    morpho_markdictionary(v, &c->methods);
    morpho_markvarrayvalue(v, &c->parents);
    morpho_markvarrayvalue(v, &c->children);
}

void objectclass_freefn(object *obj) {
    objectclass *klass = (objectclass *) obj;
    morpho_freeobject(klass->name);
    dictionary_clear(&klass->methods);
    varray_valueclear(&klass->parents);
    varray_valueclear(&klass->children);
    varray_valueclear(&klass->linearization);
}

size_t objectclass_sizefn(object *obj) {
    return sizeof(objectclass);
}

objecttypedefn objectclassdefn = {
    .printfn=objectclass_printfn,
    .markfn=objectclass_markfn,
    .freefn=objectclass_freefn,
    .sizefn=objectclass_sizefn,
    .hashfn=NULL,
    .cmpfn=NULL
};

objectclass *object_newclass(value name) {
    objectclass *newclass = (objectclass *) object_new(sizeof(objectclass), OBJECT_CLASS);

    if (newclass) {
        newclass->name=object_clonestring(name);
        dictionary_init(&newclass->methods);
        varray_valueinit(&newclass->parents);
        varray_valueinit(&newclass->children);
        newclass->superclass=NULL;
        newclass->uid=0;
    }

    return newclass;
}

/* **********************************************************************
 * objectclass utility functions
 * ********************************************************************** */

void _print(varray_value *list) {
    printf("[ ");
    for (int i=0; i<list->count; i++) {
        morpho_printvalue(NULL, list->data[i]);
        if (i<list->count-1) printf(", ");
    }
    printf(" ]");
}

/** Check if value v is in the tail of a list? */
bool _intail(varray_value *list, value v) {
    for (int i=1; i<list->count; i++) {
        if (MORPHO_ISEQUAL(list->data[i], v)) return true;
    }
    return false;
}

/** Remove value v from a list in  */
void _remove(varray_value *list, value v) {
    for (int i=0; i<list->count; i++) {
        if (MORPHO_ISEQUAL(list->data[i], v)) {
            memcpy(list->data+i, list->data+i+1, sizeof(value)*list->count-i-1);
            list->count--;
        }
    }
}

/** Check if value v is in any tail of the set of lists */
bool _inanytail(int n, varray_value *in, value v) {
    for (int i=0; i<n; i++) {
        if (_intail(&in[i], v)) return true;
    }
    return false;
}

/** Check if any of the sets contain elements */
bool _done(int n, varray_value *in) {
    for (int i=0; i<n; i++) if (in->count>0) return false;
    return true;
}

/** Performs one C3 merge operation for a set of lists  */
bool _merge(int n, varray_value *in, varray_value *out) {
    for (int i=0; i<n; i++) {
        if (in[i].count==0) continue;
        
        value head = in[i].data[0]; // Choose a head that is not in any tail
        if (_inanytail(n, in, head)) continue;

        varray_valuewrite(out, head); // Add it to the linearization and remove from the lists
        for (int j=0; j<n; j++) _remove(&in[j], head);
        return true;
    }
    return false;
}

/** Initialize the varray from the parent class's linearization */
void _init(objectclass *parent, varray_value *out) {
    if (parent->linearization.count) varray_valueadd(out, parent->linearization.data, parent->linearization.count);
}

bool _linearize(objectclass *klass, varray_value *out) {
    // Add this class to the start of the list
    varray_valuewrite(out, MORPHO_OBJECT(klass));
    
    int n = klass->parents.count;
    if (n==0) return true;
    
    // Start with the linearizations of the parent classes
    varray_value lin[n];
    for (int i=0; i<n; i++) varray_valueinit(&lin[i]);
    for (int i=0; i<n; i++) _init(MORPHO_GETCLASS(klass->parents.data[i]), &lin[i]);
    
    bool success=true;
    while (success && !_done(n, lin)) {
        success=_merge(n, lin, out);
    }

    for (int i=0; i<n; i++) varray_valueclear(&lin[i]);
    
    return success;
}

/** Computes the linearization of a class */
bool class_linearize(objectclass *klass) {
    klass->linearization.count=0;
    return _linearize(klass, &klass->linearization);
}

/* **********************************************************************
 * (Possible future) Class veneer class
 * ********************************************************************** */

//MORPHO_BEGINCLASS(Class)
//MORPHO_ENDCLASS

/* **********************************************************************
 * Initialization and finalization
 * ********************************************************************** */

objecttype objectclasstype;

void class_initialize(void) {
    // objectclass is a core type so is intialized earlier
    
    // Locate the Object class to use as the parent class of Class
    /*objectstring objname = MORPHO_STATICSTRING(OBJECT_CLASSNAME);
    value objclass = builtin_findclass(MORPHO_OBJECT(&objname));
    
    value classclass=builtin_addclass(CLASS_CLASSNAME, MORPHO_GETCLASSDEFINITION(Class), objclass);
    object_setveneerclass(OBJECT_CLASS, classclass);*/
    
    // No constructor function; classes are generated by the compiler
    
    // Class error messages
    morpho_defineerror(CLASS_INVK, ERROR_HALT, CLASS_INVK_MSG);
}
