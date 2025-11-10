import random
from prof_malloc import prof_malloc

letters='abcdefghijklmnop'

def mb(x: int) -> int:
    return x * 1024 * 1024

def gen_uniform_buffer(char: str, size: int) -> str:
    return char * size

def gen_noisy_buffer(size: int) -> str:
    return ''.join(random.choice(letters) for _ in range(size))



arr = []

@prof_malloc
def with_intern(size: int):
    arr = []
    for _ in letters:
        arr.append(gen_uniform_buffer('a', size))
    return sum(len(a) for a in arr)

        

@prof_malloc
def without_intern(size: int):
    arr = []
    for c in letters:
        arr.append(gen_uniform_buffer(c, size))
    return sum(len(a) for a in arr)



@prof_malloc
def with_noise(size: int):
    arr = []
    for _ in letters:
        arr.append(gen_noisy_buffer(size))
    return sum(len(a) for a in arr)


buffer_size = mb(1)
print(f'buffer size: {buffer_size} * {len(letters)}')

a=without_intern(buffer_size)
b=with_intern(buffer_size)
c=with_noise(buffer_size)

print(f'{a} {b} {c}')


