target triple = "i386-pc-linux-gnu"

define linkonce_odr void @outb(i32 %portw, i32 %bytew) #0 {
  %port = trunc i32 %portw to i16
  %byte = trunc i32 %bytew to i8
  call void asm sideeffect "outb  $1, $0", "{dx}N,{ax},~{flags}"(i16 %port, i8 %byte)
  ret void
}

define linkonce_odr i32 @inb(i32 %portw) #0 {
  %port = trunc i32 %portw to i16
  %byte = call i8 asm sideeffect "inb $1, $0", "={ax},{dx}N,~{flags}"(i16 %port)
  %bytew = zext i8 %byte to i32
  ret i32 %bytew
}

define linkonce_odr i32 @getCR2() #0 {
  %faultaddr = call i32 asm sideeffect "mov %cr2, $0", "=r,~{flags}"()
  ret i32 %faultaddr
}

define linkonce_odr i32 @getCR3() #0 {
  %pdir = call i32 asm sideeffect "mov %cr3, $0", "=r,~{flags}"()
  ret i32 %pdir
}

define linkonce_odr void @setCR3(i32 %pdir) #0 {
  call void asm sideeffect "  movl  $0, %cr3", "r,~{flags}"(i32 %pdir)
  ret void
}

define linkonce_odr void @invlpg(i8* %addr) #0 {
  call void asm sideeffect "invlpg ($0)", "{bx},~{memory},~{flags}"(i8* %addr)
  ret void
}

attributes #0 = { alwaysinline nounwind "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf" }
attributes #1 = { noinline optnone "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf" }
attributes #2 = { alwaysinline nounwind ssp uwtable "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf" }
