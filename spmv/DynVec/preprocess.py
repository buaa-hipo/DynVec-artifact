
with open('rearrange.dat') as f:
    rearrange = f.readlines()
with open('rearrange.cpp', 'w') as f1, open('spmv.ld', 'w') as f2, open('base.ld') as f3:
    base_ld = f3.readlines()
    base_ld = base_ld[:-1]
    f2.writelines(base_ld)
    for i, data in enumerate(rearrange):
        addr = data.split(':')[0]
        decl = f'int arr{i}[] = {{ {data.split(":")[1]} }};\n'
        f1.write(decl)
        f2.write(f'''
    . = {addr};
    arr{i} = .;
                 ''')
        
    f2.write('}')



