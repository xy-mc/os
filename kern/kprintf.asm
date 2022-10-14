[SECTION .text]

[BITS 32]

global kprintf
;===============================================
; void kprintf(u16 disp_pos, const char *format, ...)
; 参数说明：
; disp_pos: 开始打印的位置，0为0行0列，1为0行1列，80位1行0列
; format: 需要格式化输出的字符串，默认输出的字符颜色为黑底白字
; %c: 输出下一个参数的字符信息（保证参数范围在0~127），输出完打印的位置往下移动一位
; %b: 更改之后输出的字符的背景色（保证参数范围在0~15）
; %f: 更改之后输出的字符的前景色（保证参数范围在0~15）
; %s(提高内容): 参考inc/terminal.h，传进来的是一个结构体，结构体参数足够明确不复赘述，
; 输出是独立的，输出完打印的位置不会往下移动一位，不会影响接下来%c的输出的颜色
; 其余字符：按照字符输出（保证字符里不会有%，\n等奇奇怪怪的字符，都是常见字符，%后面必会跟上述三个参数之一），输出完打印的位置往下移动一位
kprintf:
	push ebp
	mov  ebp,esp
	pusha
	mov  edi,[ebp+8]
	mov  esi,[ebp+12]
	shl  edi,1
	mov  ah,0xf
	mov  ecx,16
.1:
        lodsb
        test al,al
        jz   .2
        cmp  al,'%'
        jz   .1
        cmp  al,'c'
        jz   .3
        cmp  al,'b'
        jz   .4
        cmp  al,'f'
        jz   .5
        mov [gs:edi],ax
        add edi,2
        jmp  .1
.3:
        mov al,[ebp+ecx]
        add ecx,4
        mov [gs:edi],ax
        add edi,2
        jmp .1
.4:
        mov dl,[ebp+ecx]
        add ecx,4
        and ah,0xf
        shl dl,4
        or  ah,dl
        jmp .1
.5:
        mov dl,[ebp+ecx]
        add ecx,4
        and ah,0xf0
        or  ah,dl
        jmp .1
.2:
        popa
        pop ebp
        ret
       
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
