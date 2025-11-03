def f(): pass

class A():
    def f(self): pass
    class A():
        def f(self): pass

print(f.__name__)
print(A.f.__name__)
print(A.A.f.__name__)

print(f.__qualname__)
print(A.f.__qualname__)
print(A.A.f.__qualname__)
