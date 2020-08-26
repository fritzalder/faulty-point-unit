.data
my_vec:
.space 16
my_fpucw:
.space 16

.text
.global get_fpucw
get_fpucw:
    FSTCW my_fpucw(%RIP)
    MOV my_fpucw(%RIP), %RAX
    RET

.global add_fpu
# \arg %xmm0: num
# \res %xmm0: 2*num
add_fpu:
    movsd %xmm0, my_vec(%rip)
    fldl my_vec(%rip)
    fadd %st, %st
    fstl my_vec(%rip)
    movsd my_vec(%rip),%xmm0
    ret
