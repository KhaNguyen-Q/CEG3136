/*
 * maths.s
 *
 *  Created on: Nov 3, 2025
 *      Author: knguy138
 */
.syntax unified
.cpu cortex-m33
.thumb

.section .text

// uint32_t Increment(uint32_t num);
.global Increment
.type Increment, %function
Increment:
add r0, r0, #1
bx lr

// uint32_t Decrement(uint32_t num);
.global Decrement
.type Increment, %function
Decrement:
sub r0, r0, #1
bx lr

// basic 4 functions uint32_t(uint32_t sel, uint32_t a, uint32_t b)
// sel=0 is add, sel=1 is sub, sel=2 is mult, sel=3 is div
// written from scratch
.global Classic4Function
.type Classic4Function, %function
Classic4Function:
cmp r0, #0
itt eq
addeq r0, r1, r2
bxeq lr
cmp r0, #1
itt eq
subeq r0, r1, r2
bxeq lr
cmp r0, #2
itt eq
muleq r0, r1, r2
bxeq lr
cmp r0, #3
itt eq
sdiveq r0, r1, r2
bxeq lr


//GCD Functions uint32_t(uint32_t a, uint32_t b)
// taken from lecture 7 slide 29
.global GCD
.type GCD, %function
GCD:
cmp r0, r1
beq END
blo LESS
sub r0, r0, r1
b GCD
LESS:
sub r1, r1, r0
b GCD
END:
bx lr

//factorial uint32_t(uint32_t a)
// written from scratch
.global Factorial
.type Factorial, %function
Factorial:
cmp r0, #2
itt lt //if n<2
movlt r0, #1
bxlt lr //return
mov r1, r0 //r1 has n
sub r0, r0, #1 // r0 has n-1
push {lr, r1}// store lr and r1
bl Factorial// call function again
pop {lr, r1}
mul r0, r1, r0 //return value
bx lr//go back to original function

// fibbonacci uint32_t(uint32_t a)
// written from scratch
.global Fibbonacci
.type Fibbonacci, %function
Fibbonacci:
push {r4}
cmp r0, #0
it eq
bxeq lr
cmp r0, #1
itt eq
moveq r0, #1
bxeq lr// return 1 if n=1
mov r1, #1
mov r2, #0 //1 - 3 numbers for fibb
mov r3, #1 //2 - ||
mov r4, #0 //3 - ||
Loop:
cmp r1, r0
it eq
beq Done //check if done and return
add r4, r3, r2 //add r3 and r2 to r4
mov r2, r3 //set previous numbers
mov r3, r4 //set previous numbers
add r1, r1, #1
b Loop
Done:
mov r0, r4 //return r4
pop {r4}
bx lr


// uint32_t Sort(uint32_t n, uint32_t *arr)
// AI generated
.global Sort
.type Sort, %function
Sort:
    push    {r4, r5, r6, r7, lr}     // Save registers we will use
    mov     r2, #0                   // i = 0 (outer loop counter)
    subs    r3, r0, #1               // r3 = n-1 (loop max)
    ble     sort_done                // If n-1 <= 0, skip sorting
outer_loop:
    mov     r4, #0                   // j = 0 (inner loop)
    subs    r5, r3, r2               // r5 = n-1-i
    ble     next_outer               // If n-1-i <= 0, outer+1
inner_loop:
    // Calculate addresses for arr[j] and arr[j+1]
    lsl     r6, r4, #2               // r6 = j * 4, offset for arr[j]
    add     r7, r1, r6               // r7 = &arr[j]
    ldr     r8, [r7]                 // r8 = arr[j]
    add     r9, r7, #4               // &arr[j+1] = &arr[j] + 4
    ldr     r10, [r9]                // r10 = arr[j+1]
    cmp     r8, r10                  // if arr[j] > arr[j+1]?
    bls     no_swap                  // if not, skip swap
    // Swap arr[j] and arr[j+1]
    str     r10, [r7]                // arr[j] = arr[j+1]
    str     r8, [r9]                 // arr[j+1] = arr[j]
no_swap:
    add     r4, r4, #1               // j++
    cmp     r4, r5                   // if j < n-1-i
    blt     inner_loop
next_outer:
    add     r2, r2, #1               // i++
    cmp     r2, r3                   // if i < n-1
    blt     outer_loop
sort_done:
    pop     {r4, r5, r6, r7, lr}
    bx      lr                       // Return, n already in r0



//average  uint32_t(uint32_t length, uint32_t *arr)
//written from scratch
.global Average
.type Average, %function
Average:
cmp r0, #0 //check for div by 0 error
it eq
bxeq lr
push {r4, r5}
mov r2, #0 //r2 holds the sum
mov r3, #0 //iterator i
avg_loop:
cmp r3, r0
beq avg_done //finish loop if i=n
lsl r4, r3, #2 //multiply i*4
add r4, r1, r4 //get actual memory address in r4
ldr r5, [r4] //load from memory into r5 where we are at
add r2, r2, r5 // sum += arr[i]
add r3, r3, #1 //i++
b avg_loop
avg_done:
pop {r4, r5}
udiv r0, r2, r0
bx lr

//compare two numbers uint32_t(uint32_t a, uint32_t b)
.global Compare
.type Compare, %function
Compare:
cmp r0, r1
it eq
moveq r0, #0
it lt
movlt r0, #1
it gt
movgt r0, #2
bx lr
