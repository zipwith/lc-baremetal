target triple = "i386-pc-linux-gnu"

define void @outb(i32 %portw, i32 %bytew) #0 {
  %port = trunc i32 %portw to i16
  %byte = trunc i32 %bytew to i8
  call void asm sideeffect "outb  $1, $0", "{dx}N,{ax},~{flags}"(i16 %port, i8 %byte)
  ret void
}

define i32 @inb(i32 %portw) #0 {
  %port = trunc i32 %portw to i16
  %byte = call i8 asm sideeffect "inb $1, $0", "={ax},{dx}N,~{flags}"(i16 %port)
  %bytew = zext i8 %byte to i32
  ret i32 %bytew
}

attributes #0 = { noinline optnone "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf" }
