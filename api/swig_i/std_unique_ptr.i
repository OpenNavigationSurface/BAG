// https://stackoverflow.com/questions/27693812/how-to-handle-unique-ptrs-with-swig
// "You can add or remove constructors depending on exactly what semantics you want 
// within Python, I overlooked the custom deleters here."

namespace std {
  %feature("novaluewrapper") unique_ptr;
  template <typename Type>
  struct unique_ptr {
     typedef Type* pointer;

     explicit unique_ptr( pointer __p );
     unique_ptr (unique_ptr&& __u);
     template<class Type2, Class Del2> unique_ptr( unique_ptr<Type2, Del2>&& __u );
     unique_ptr( const unique_ptr& __u) = delete;
     ~unique_ptr();

     pointer operator-> () const;
     void reset (pointer __p=pointer());
     void swap (unique_ptr &__u);
     explicit operator bool () const;

     // disabled because they are shadowed by UInt8Array methods
     //pointer release ();
     //pointer get () const;
  };
}

%define wrap_unique_ptr(Name, Type)
  %template(Name) std::unique_ptr<Type>;
  %newobject std::unique_ptr<Type>::release;

  %typemap(out) std::unique_ptr<Type> %{
    $result = SWIG_NewPointerObj(new $1_ltype(std::move($1)), $&1_descriptor, SWIG_POINTER_OWN);
  %}

%enddef
