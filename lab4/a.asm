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

func:
subu $sp, $sp, 8
sw $ra, 4($sp)
sw $fp, 0($sp)
addi $fp, $sp, 8
move $s7, $a0
move $s6, $a1
move $s5, $a2
move $s4, $a3
lw $s3, 8($sp)
lw $s2, 12($sp)
subu $v1, $fp, 32
sw $s2, 0($v1)
subu $v1, $fp, 28
sw $s3, 0($v1)
subu $v1, $fp, 24
sw $s4, 0($v1)
subu $v1, $fp, 20
sw $s5, 0($v1)
subu $v1, $fp, 16
sw $s6, 0($v1)
subu $v1, $fp, 12
sw $s7, 0($v1)
subu $v1, $fp, 12
lw $s7, 0($v1)
move $a0, $s7
jal write
subu $v1, $fp, 12
sw $s7, 0($v1)
subu $v1, $fp, 16
lw $s7, 0($v1)
move $a0, $s7
jal write
subu $v1, $fp, 16
sw $s7, 0($v1)
subu $v1, $fp, 20
lw $s7, 0($v1)
move $a0, $s7
jal write
subu $v1, $fp, 20
sw $s7, 0($v1)
subu $v1, $fp, 24
lw $s7, 0($v1)
move $a0, $s7
jal write
subu $v1, $fp, 24
sw $s7, 0($v1)
subu $v1, $fp, 28
lw $s7, 0($v1)
move $a0, $s7
jal write
subu $v1, $fp, 28
sw $s7, 0($v1)
subu $v1, $fp, 32
lw $s7, 0($v1)
move $a0, $s7
jal write
li $s6, 0
subu $sp, $fp, 8
lw $ra, 4($sp)
lw $fp, 0($sp)
li $v0, 0
jr $ra

main:
subu $sp, $sp, 8
sw $ra, 4($sp)
sw $fp, 0($sp)
addi $fp, $sp, 8
subu $sp, $fp, 16
li $s7, 1
move $a0, $s7
li $s6, 2
move $a1, $s6
li $s5, 3
move $a2, $s5
li $s4, 4
move $a3, $s4
li $s3, 5
sw $s3, 0($sp)
li $s2, 6
sw $s2, 4($sp)
jal func
move $s1, $v0
li $s0, 0
subu $sp, $fp, 8
lw $ra, 4($sp)
lw $fp, 0($sp)
li $v0, 0
jr $ra
