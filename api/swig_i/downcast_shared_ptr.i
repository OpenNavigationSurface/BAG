// Downcasting for shared_ptr
// Taken from http://swig.10945.n7.nabble.com/Inheritance-with-smart-pointers-doesn-t-work-tp6741p6749.html

// Some macros for better readability
// %ptr(TYPE) -> std::tr1::shared_ptr<TYPE> (or boost::shared_ptr<TYPE> // or std::shared_ptr<TYPE>)
// %dynptrcast(TYPE,INPUT) -> std::tr1::dynamic_pointer_cast<TYPE>(INPUT)

#define %ptr(Type) SWIG_SHARED_PTR_QNAMESPACE::shared_ptr<Type>
#define %dynptrcast(Type, input) \
    SWIG_SHARED_PTR_QNAMESPACE::dynamic_pointer_cast<Type>(input)


// Helper function for %downcast, to be called for each target type
%define %_downcast(Type)
    if (dcast == 0 && $1) { // check for Type
        %ptr(Type) out = %dynptrcast(Type, $1);

        if (out) { // out is Type
            dcast = 1;

            %set_output(SWIG_NewPointerObj(new %ptr(Type)(out),
                $descriptor(%ptr(Type)*), SWIG_POINTER_OWN));
        }
    }
%enddef

// Macro to be used.
// Usage:  %downcast(function/method, Derived1, Derived2, .., Base)
//      Note, order is important for the derived parameters!
%define %downcast_shared(typemaptarget, Types...)
    %typemap(out) typemaptarget {
        int dcast = 0;

        %formacro(%_downcast, Types)
        if (!dcast)
            SWIG_exception_fail(SWIG_TypeError,"Can't convert type");
    }
%enddef

