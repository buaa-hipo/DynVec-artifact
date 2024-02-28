
with open('rearrange.dat') as f:
    rearrange = f.readlines()
with open('rearrange.cpp', 'w') as f1, open('spmv.ld', 'w') as f2, open('base.ld') as f3:
    f1.write('#include <string.h>\n')
    base_ld = f3.readlines()
    base_ld = base_ld[:-1]
    f2.writelines(base_ld)
    cpy = ''
    for i, data in enumerate(rearrange):
        addr = data.split(':')[0]
        decl = f'''int arr{i}[] = {{ {data.split(":")[1]} }};
        int carr{i}[] = {{ {data.split(":")[1]} }};
                '''
        f1.write(decl)
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



