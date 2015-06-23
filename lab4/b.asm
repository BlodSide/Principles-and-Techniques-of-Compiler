.data
_prompt: .asciiz "Enter an integer:"
_ret: .asciiz "\n"
.globl main
.text

read:
subu $sp, $sp, 8
sw $ra, 4($sp)
sw $fp, 0($sp)
addi $fp, $sp, 8
li $v0, 4
la $a0, _prompt
syscall
li $v0, 5
syscall
subu $sp, $fp, 8
lw $ra, 4($sp)
lw $fp, 0($sp)
jr $ra

write:
subu $sp, $sp, 8
sw $ra, 4($sp)
sw $fp, 0($sp)
addi $fp, $sp, 8
li $v0, 1
syscall
li $v0, 4
la $a0, _ret
syscall
subu $sp, $fp, 8
lw $ra, 4($sp)
lw $fp, 0($sp)
move $v0, $0
jr $ra

fib:
subu $sp, $sp, 8
sw $ra, 4($sp)
sw $fp, 0($sp)
addi $fp, $sp, 8
move $s7, $a0
subu $v1, $fp, 12
sw $s7, 0($v1)
subu $v1, $fp, 12
lw $s7, 0($v1)
li $s6, 0
bne $s7, $s6, label2
li $s5, 1
subu $sp, $fp, 8
lw $ra, 4($sp)
lw $fp, 0($sp)
li $v0, 1
jr $ra
subu $v1, $fp, 12
sw $s7, 0($v1)
label2:
subu $v1, $fp, 12
lw $s7, 0($v1)
li $s4, 1
bne $s7, $s4, label5
li $s3, 1
subu $sp, $fp, 8
lw $ra, 4($sp)
lw $fp, 0($sp)
li $v0, 1
jr $ra
subu $v1, $fp, 12
sw $s7, 0($v1)
label5:
subu $v1, $fp, 12
lw $s7, 0($v1)
li $s2, 1
sub $s1, $s7, $s2
subu $v1, $fp, 16
sw $s1, 0($v1)
subu $v1, $fp, 12
sw $s7, 0($v1)
subu $sp, $fp, 16
subu $v1, $fp, 16
lw $s7, 0($v1)
move $a0, $s7
jal fib
move $s1, $v0
subu $v1, $fp, 12
lw $s0, 0($v1)
li $t9, 2
sub $t8, $s0, $t9
subu $v1, $fp, 24
sw $t8, 0($v1)
subu $v1, $fp, 20
sw $s1, 0($v1)
subu $v1, $fp, 16
sw $s7, 0($v1)
subu $v1, $fp, 12
sw $s0, 0($v1)
subu $sp, $fp, 24
subu $v1, $fp, 24
lw $s7, 0($v1)
move $a0, $s7
jal fib
move $s1, $v0
subu $v1, $fp, 20
lw $s0, 0($v1)
add $t8, $s0, $s1
subu $sp, $fp, 8
lw $ra, 4($sp)
lw $fp, 0($sp)
move $v0, $t8
jr $ra

main:
subu $sp, $sp, 8
sw $ra, 4($sp)
sw $fp, 0($sp)
addi $fp, $sp, 8
subu $sp, $fp, 8
li $s7, 5
move $a0, $s7
jal fib
move $s6, $v0
subu $sp, $fp, 12
move $a0, $s6
jal write
subu $v1, $fp, 12
sw $s6, 0($v1)
subu $sp, $fp, 12
li $s6, 4
move $a0, $s6
jal fib
move $s5, $v0
subu $sp, $fp, 16
move $a0, $s5
jal write
subu $v1, $fp, 16
sw $s5, 0($v1)
subu $sp, $fp, 16
li $s5, 3
move $a0, $s5
jal fib
move $s4, $v0
subu $sp, $fp, 20
move $a0, $s4
jal write
subu $v1, $fp, 20
sw $s4, 0($v1)
subu $sp, $fp, 20
li $s4, 2
move $a0, $s4
jal fib
move $s3, $v0
subu $sp, $fp, 24
move $a0, $s3
jal write
subu $v1, $fp, 24
sw $s3, 0($v1)
subu $sp, $fp, 24
li $s3, 1
move $a0, $s3
jal fib
move $s2, $v0
subu $sp, $fp, 28
move $a0, $s2
jal write
li $s1, 0
subu $sp, $fp, 8
lw $ra, 4($sp)
lw $fp, 0($sp)
li $v0, 0
jr $ra
