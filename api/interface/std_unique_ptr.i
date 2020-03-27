// https://stackoverflow.com/questions/27693812/how-to-handle-unique-ptrs-with-swig
// "You can add or remove constructors depending on exactly what semantics you want 
// within Python, I overlooked the custom deleters here."

namespace std {
    %feature("novaluewrapper") unique_ptr;
    template <typename Type>
    struct unique_ptr {
        typedef Type* pointer;

        explicit unique_ptr(pointer __p);
        unique_ptr(unique_ptr&& __u);
        template<class Type2, Class D> unique_ptr(unique_ptr<U, D>&& __u);
        unique_ptr(const unique_ptr& __u) = delete;
        ~unique_ptr();

        /// are op= needed?
       // unique_ptr& operator=(unique_ptr&& __r);
        //template< class U, class E > unique_ptr& operator=(unique_ptr<U,E>&& __r);
        //unique_ptr& operator=(nullptr_t);

       // typename std::add_lvalue_reference<Type>::type operator*() const;
        pointer operator-> () const;
        pointer get() const;
        explicit operator bool() const;

        pointer release();
        void reset (pointer __p=pointer());
        void swap (unique_ptr& __u);


        //members of the specialization unique_ptr<T[]> -- needed?
        template<class U> void reset(U __u) = delete;
        void reset(std::nullptr_t __p);
        //Type& operator[](size_t i) const;
    };




    ///// are these needed?

    //(only for non-array types)
    template< class T, class... Args > unique_ptr<T> make_unique( Args&&... args );

    //(only for array types with unknown bound)
    template< class T > unique_ptr<T> make_unique( std::size_t size );

    //(only for array types with known bound)
    template< class T, class... Args > /* unspecified */ make_unique( Args&&... args ) = delete;

}

%define wrap_unique_ptr(Name, Type)
    %template(Name) std::unique_ptr<Type>;
    %newobject std::unique_ptr<Type>::release;

    %typemap(out, optimal="1") std::unique_ptr<Type>, std::unique_ptr<Type>& %{   
        $result = SWIG_NewPointerObj(new $1_ltype(std::move($1)), $&1_descriptor, SWIG_POINTER_OWN);
    %}

%enddef