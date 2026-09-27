# 데모 GIF 녹화 스크립트 (https://github.com/charmbracelet/vhs)
# 프로젝트 루트에서 실행: vhs assets/demo.tape

Output assets/demo.gif

Set Shell "bash"
Set FontSize 18
Set Width 960
Set Height 620
Set Padding 20
Set TypingSpeed 55ms

Hide
Type "export PS1='$ ' && make -s && clear"
Enter
Sleep 2s
Show

Type "./calculator"
Enter
Sleep 1s

Type "2+3*4"
Enter
Sleep 700ms
Type "(2+3)*4"
Enter
Sleep 700ms
Type "2^3^2"
Enter
Sleep 700ms
Type "x = 10"
Enter
Sleep 700ms
Type "x * 2 + ans"
Enter
Sleep 700ms
Type "sin(pi / 2)"
Enter
Sleep 700ms
Type "log(0)"
Enter
Sleep 900ms
Type "1+2)"
Enter
Sleep 900ms
Type "history"
Enter
Sleep 2s
Type "quit"
Enter
Sleep 2s

Hide
Type "make clean"
Enter
