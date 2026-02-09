# C++ CLI Calculator

간단한 명령줄 인터페이스(CLI) 계산기입니다.

## 기능

- 기본 사칙연산 지원 (+, -, *, /)
- 실수 연산 지원
- 0으로 나누기 방지
- 대화형 입력 모드

## 빌드 방법

### CMake 사용
```bash
mkdir build
cd build
cmake ..
make
./bin/calculator
```

### Makefile 사용
```bash
make
./calculator
```

### 직접 컴파일
```bash
g++ -std=c++17 -Wall -Wextra -O2 -o calculator main.cpp
./calculator
```

## 사용 방법

프로그램을 실행한 후 다음과 같이 입력하세요:

```
> 5 + 3
Result: 8

> 10.5 * 2
Result: 21

> 15 / 3
Result: 5

> quit
```

## 예제

```
=== C++ CLI Calculator ===
Enter expressions like: 5 + 3
Type 'quit' or 'exit' to exit

> 10 + 20
Result: 30
> 50 - 15
Result: 35
> 6 * 7
Result: 42
> 100 / 4
Result: 25
> quit
Goodbye!
```

## 향후 개선 사항

- [ ] 괄호 지원
- [ ] 복잡한 수식 파싱
- [ ] 변수 저장 기능
- [ ] 계산 히스토리
- [ ] 과학 계산 함수 (sin, cos, log 등)
