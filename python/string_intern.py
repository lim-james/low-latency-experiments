from sys import intern

from prof_malloc import prof_malloc

base_str = "testing out the results"

def strcpy(s:str) -> str:
    return ''.join(c for c in s)

@prof_malloc
def with_intern(count: int):
    interned_str = intern(strcpy(base_str))
    arr = []
    for _ in range(count):
        arr.append(interned_str)
    return arr

@prof_malloc
def without_intern(count: int):
    arr = []
    for _ in range(count):
        arr.append(strcpy(base_str))
    return arr

sample_count = 1_000_000

a=with_intern(sample_count)
b=without_intern(sample_count)

base_str_len = len(base_str)
expected_size_b = sample_count * base_str_len

print('--------------- Validating ---------------')
print(f'results match          : {a==b}')
print(f'expected mem usage (b) : {expected_size_b}')
print('------------------------------------------')
