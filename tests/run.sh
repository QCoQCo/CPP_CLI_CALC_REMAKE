#!/bin/bash
set -e

# 프로젝트 루트 디렉터리로 이동
cd "$(dirname "$0")/.."

echo "=== Building calculator ==="
make

CALC="./calculator"
FAILED=0
TOTAL=0

run_test() {
    local name="$1"
    local input="$2"
    local expected_pattern="$3"
    local should_fail="${4:-false}"
    
    TOTAL=$((TOTAL + 1))
    
    # 2초 타임아웃을 걸어 무한 루프 방지
    local output
    if ! output=$(python3 -c "
import subprocess, sys
p = subprocess.Popen(['$CALC'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
try:
    out, _ = p.communicate(input='''$input''', timeout=2)
    sys.stdout.write(out)
except subprocess.TimeoutExpired:
    p.kill()
    sys.exit(124)
" 2>&1); then
        local ret=$?
        if [ $ret -eq 124 ]; then
            echo "❌ [$name] FAIL (Timeout / Infinite loop)"
            FAILED=$((FAILED + 1))
            return
        fi
    fi

    if echo "$output" | grep -E "$expected_pattern" > /dev/null; then
        echo "✅ [$name] PASS"
    else
        echo "❌ [$name] FAIL"
        echo "   Input: $input"
        echo "   Expected pattern: $expected_pattern"
        echo "   Actual output: $output"
        FAILED=$((FAILED + 1))
    fi
}

echo ""
echo "=== Running Tests ==="

# 1. 기존 정상 동작 테스트 (회귀 방지)
run_test "Basic Addition" "5+3\nquit\n" "Result: 8"
run_test "Operator Precedence" "(1+2)*4\nquit\n" "Result: 12"
run_test "Variable Assignment" "x = 10\nx * 2\nquit\n" "Result: 20"
run_test "Ans Support" "10 + 5\nans * 2\nquit\n" "Result: 30"
run_test "Function sin" "sin(0)\nquit\n" "Result: 0"
run_test "Function sqrt" "sqrt(16)\nquit\n" "Result: 4"
run_test "Function pow" "pow(2,3)\nquit\n" "Result: 8"

# 2. B1: EOF 무한 루프 검증 (파이프 입력 시 정상 종료)
run_test "B1: EOF Handling" "1+1\n" "Result: 2"

# 3. B2: 후행 토큰 에러 검증
run_test "B2: Trailing RPAREN" "1+2)\nquit\n" "Error:"
run_test "B2: Trailing COMMA" "1,2\nquit\n" "Error:"
run_test "B2: Trailing ASSIGN in expr" "1+2=5\nquit\n" "Error:"
run_test "B2: Trailing RPAREN in assign" "x=5)\nvars\nquit\n" "Error:"

# 4. B3: 공백 처리 검증
run_test "B3: Separated numbers error" "1 2\nquit\n" "Error:"
run_test "B3: Spaces in assignment" "x = 10\nquit\n" "Result: 10"
run_test "B3: Leading and trailing spaces" " 5 + 3 \nquit\n" "Result: 8"

# 5. B4: 잘못된 숫자 형식 검증
run_test "B4: Multiple dots in number" "1.2.3\nquit\n" "Error:"
run_test "B4: Leading dot number" ".5\nquit\n" "Result: 0.5"

# 6. B5: 명령어 공백 처리 검증
run_test "B5: Leading space quit" " quit\n" "Goodbye!"
run_test "B5: Space in command should not match" "his tory\nquit\n" "Error:"

# 7. B6: UTF-8 멀티바이트 안전성 검증
run_test "B6: UTF-8 character error" "x=3한\nquit\n" "Error:"

# 8. P3 개선 사항 검증
run_test "I1: Domain error log(0)" "log(0)\nquit\n" "Error:"
run_test "I1: Domain error log(-1)" "log(-1)\nquit\n" "Error:"
run_test "I1: Domain error 0^-1" "0^-1\nquit\n" "Error:"
run_test "I1: Overflow multiply" "1e308*10\nquit\n" "Error:"
run_test "I1: Overflow add" "1e308+1e308\nquit\n" "Error:"
run_test "I1: Overflow in subexpression" "1/(1e308*10)\nquit\n" "Error:"
run_test "I2: High precision" "1234567\nquit\n" "Result: 1234567"
run_test "I3: Scientific notation" "1e5\nquit\n" "Result: 100000"
run_test "I3: Scientific notation with sign" "1.5e-3\nquit\n" "Result: 0.0015"
run_test "I4: Built-in constants" "pi\nquit\n" "Result: 3.14159"
run_test "I4: Built-in constant e" "e\nquit\n" "Result: 2.71828"
run_test "I5: Reserved word assignment error" "sin=3\nquit\n" "Error:"
run_test "I5: Reserved command assignment error" "vars=5\nquit\n" "Error:"
run_test "I6: Informative error token" "1+*2\nquit\n" "Error: Unexpected token: \*"

echo ""
echo "=== Test Summary ==="
echo "Total: $TOTAL, Failed: $FAILED"

if [ $FAILED -ne 0 ]; then
    exit 1
fi
