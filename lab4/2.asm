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

fact:
subu $sp, $sp, 8
sw $ra, 4($sp)
sw $fp, 0($sp)
addi $fp, $sp, 8
move $s7, $a0
subu $v1, $fp, 12
sw $s7, 0($v1)
subu $v1, $fp, 12
lw $s7, 0($v1)
li $s6, 1
bne $s7, $s6, label2
subu $sp, $fp, 8
lw $ra, 4($sp)
lw $fp, 0($sp)
move $v0, $s7
jr $ra
subu $v1, $fp, 12
sw $s7, 0($v1)
label2:
subu $v1, $fp, 12
lw $s7, 0($v1)
li $s5, 1
sub $s4, $s7, $s5
subu $v1, $fp, 16
sw $s4, 0($v1)
subu $v1, $fp, 12
sw $s7, 0($v1)
subu $sp, $fp, 16
subu $v1, $fp, 16
lw $s7, 0($v1)
move $a0, $s7
jal fact
move $s4, $v0
subu $v1, $fp, 12
lw $s3, 0($v1)
mul $s2, $s3, $s4
subu $sp, $fp, 8
lw $ra, 4($sp)
lw $fp, 0($sp)
move $v0, $s2
jr $ra

main:
subu $sp, $sp, 8
sw $ra, 4($sp)
sw $fp, 0($sp)
addi $fp, $sp, 8
subu $sp, $fp, 8
jal read
move $s7, $v0
subu $v1, $fp, 12
sw $s7, 0($v1)
subu $v1, $fp, 12
lw $s7, 0($v1)
li $s6, 1
ble $s7, $s6, label5
subu $v1, $fp, 12
sw $s7, 0($v1)
subu $sp, $fp, 12
subu $v1, $fp, 12
lw $s7, 0($v1)
move $a0, $s7
jal fact
move $s5, $v0
subu $v1, $fp, 16
sw $s5, 0($v1)
subu $v1, $fp, 12
sw $s7, 0($v1)
j label6
label5:
subu $v1, $fp, 16
lw $s7, 0($v1)
li $s5, 1
move $s7, $s5
subu $v1, $fp, 16
sw $s7, 0($v1)
label6:
subu $sp, $fp, 16
subu $v1, $fp, 16
lw $s7, 0($v1)
move $a0, $s7
jal write
li $s4, 0
subu $sp, $fp, 8
lw $ra, 4($sp)
lw $fp, 0($sp)
li $v0, 0
jr $ra
