target triple = "i386-pc-linux-gnu"

define i32 @getCR2() #0 {
  %faultaddr = call i32 asm sideeffect "mov %cr2, $0", "=r,~{flags}"()
  ret i32 %faultaddr
}

define i32 @getCR3() #0 {
  %pdir = call i32 asm sideeffect "mov %cr3, $0", "=r,~{flags}"()
  ret i32 %pdir
}

define void @setCR3(i32 %pdir) #0 {
  call void asm sideeffect "  movl  $0, %cr3", "r,~{flags}"(i32 %pdir)
  ret void
}

define void @invlpg(i8* %addr) #0 {
  call void asm sideeffect "invlpg ($0)", "{bx},~{memory},~{flags}"(i8* %addr)
  ret void
}

attributes #0 = { noinline optnone "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf" }
