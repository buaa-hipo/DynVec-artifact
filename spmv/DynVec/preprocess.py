
with open('rearrange.dat') as f:
    rearrange = f.readlines()
with open('rearrange.cpp', 'w') as f1, open('spmv.ld', 'w') as f2, open('base.ld') as f3:
    f1.write('#include <string.h>\n')
    f1.write('#include <stdio.h>\n')
    f1.write('#include <stdint.h>\n')
    f1.write('#include <sys/mman.h>\n')

    base_ld = f3.readlines()
    base_ld = base_ld[:-1]
    f2.writelines(base_ld)
    cpy = ''
    for i, data in enumerate(rearrange):
        addr = data.split(':')[0]
        decl = f'''extern int arr{i}[]; 
        int carr{i}[] = {{ {data.split(":")[1]} }};
                '''
        # decl = f'''int arr{i}[] = {{ {data.split(":")[1]} }};
        # int carr{i}[] = {{ {data.split(":")[1]} }};
        #         '''
        f1.write(decl)
        # cpy += f'mmap((void*)(((uint64_t)arr{i})&(~0xfffL)), (sizeof(carr{i}) & (~0xfff)) + 4096, PROT_WRITE | PROT_READ, MAP_ANONYMOUS, -1, 0 );\n'
        cpy += f'memcpy(arr{i}, carr{i}, sizeof(carr{i}));\n'
        f2.write(f'''
    . = {addr};
    arr{i} = .;
                 ''')
    f1.write(f'''
class _Init {{
public:
    _Init() {{
{cpy}
}}
}};
_Init _i;
''')
    f2.write('}')



