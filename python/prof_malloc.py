import tracemalloc
from functools import wraps 

def prof_malloc(fn):
    @wraps(fn)
    def decorator(*args, **kwargs):
        tracemalloc.start()

        ret = fn(*args, **kwargs)

        current_size, peak_size = tracemalloc.get_traced_memory()
        tracemalloc.stop()

        print(f'----- malloc stats for {fn.__name__} -----')
        print(f'current size (kb) : {current_size}')
        print(f'peak size (kb)    : {peak_size}')
        print('------------------------------------------')

        return ret

    return decorator


