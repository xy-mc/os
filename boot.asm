    org    0x7c00            ; 告诉编译器程序加载到7c00处
    mov    ax, cs
    mov    ds, ax
    mov    es, ax
    call   DispStr           ; 调用显示字符串例程
    jmp    $                 ; 无限循环
DispStr:
    mov    ax, BootMessage
    mov    bp, ax            ; ES:BP = 串地址
    mov    cx, 23            ; CX = 串长度
    mov    ax, 01301h        ; AH = 13,  AL = 01h
    mov    bx, 000ch         ; 页号为0(BH = 0) 黑底红字(BL = 0Ch,高亮)
    mov    dl, 0;
    int    10h;
    mov    ah,06h;
    mov    al,0;
    mov    ch,0;
    mov    cl,0;
    mov    dh,50h;
    mov    dl,50h;
    int    10h;
    mov    ah,02h;
    mov    dh,13h;
    mov    dl,26h;
    mov    bh,0;
    int    10h;
    mov    ax,BootMessage1
    mov    bp,ax;
    mov    ah,13h;
    mov    bx,00f9h;
    mov    cx,4;
    int    10h;
    mov    ah,02h;
    mov    dl,0;
    mov    dh,0;
    mov    dl,0;
    int    10h;
    mov    ax,M2
    mov    bp,ax;
    mov    cx,2;
    mov    ax,01300h;
    mov    bx,0004h;
    mov    dl,0;
    int    10h               ; 10h 号中断
    ret
BootMessage:             db    "This is luoqiang's boot"
BootMessage1:            db    "NWPU"
M2:                      db    "    "
times      510-($-$$)    db    0    ; 填充剩下的空间，使生成的二进制代码恰好为512字节
dw         0xaa55                   ; 结束标志
