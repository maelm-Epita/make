#!/bin/sh

testdir="$(pwd)/tests"
program="$(pwd)/minimake"

cd "${testdir}" || exit

RED='\033[0;31m'
GREEN='\033[0;32m'
NOC='\033[0m'

passed=0
total=0

print_test_result()
{
    if [ "${actual}" = "${expected}" ]; then
        echo -e "${GREEN}Test passed${NOC}"
        passed=$(( passed + 1 ))
    else
        echo -e "${RED}Test failed${NOC}"
        diff <(echo "$actual") <(echo "$expected")
    fi
    total=$(( total + 1 ))
}

test_stdout_file_print()
{
    actual=$(${program} -p -f "testfiles/$1")
    expected=$(cat "expected/$1")
    print_test_result "${actual}" "${expected}"
}

test_stdout_file()
{
    actual=$(${program} -f "testfiles/$1")
    expected=$(cat "expected/$1")
    print_test_result "${actual}" "${expected}"
}

test_err()
{
    actual=$(eval "$1" 2>&1)
    expected=$2
    print_test_result "${actual}" "${expected}"
}

test_stdout()
{
    actual=$(eval "$1")
    expected=$2
    print_test_result "${actual}" "${expected}"
}

echo "Argument errors -------"
test_err "${program} -f" "minimake: *** no filename provided.  Stop."
test_err "${program} \"\"" "minimake: *** empty string invalid as argument.  Stop."

echo "Parsing errors --------"
echo "Unterminated variable reference"
test_err "${program} -f testfiles/parser-err/Makefile-unterminated" "minimake: *** unterminated variable reference.  Stop."
echo "Empty variable name"
test_err "${program} -f testfiles/parser-err/Makefile-empty-var-name" "minimake: *** empty variable name.  Stop."
echo "Variable no value"
test_err "${program} -f testfiles/parser-err/Makefile-var-no-value" "minimake: *** variable definition with no value.  Stop."
echo "Command outside rule"
test_err "${program} -f testfiles/parser-err/Makefile-command-outside-rule" "minimake: *** found command outside of rule.  Stop."
echo "Missing separator"
test_err "${program} -f testfiles/parser-err/Makefile-missing-sep" "minimake: *** missing separator.  Stop."

echo "Building errors -------"
echo "No rule to make target"
test_err "${program} -f testfiles/builder-err/Makefile-no-rule hello" "minimake: *** No rule to make target 'hello'.  Stop."
echo "No rule to make target needed by"
test_err "${program} -f testfiles/builder-err/Makefile-no-rule-needed-by hello" "minimake: *** No rule to make target 'test', needed by 'hello'.  Stop."

echo "Execution errors ------"
echo "Unterminated variable reference in recipe"
test_err "${program} -f testfiles/execution-err/Makefile-unterminated" "minimake: *** unterminated variable reference.  Stop."
echo "Command failure"
test_err "${program} -f testfiles/execution-err/Makefile-bad-command falseRule" "minimake: *** command returned with non 0 code '1'.  Stop."
test_err "${program} -f testfiles/execution-err/Makefile-bad-command notACommandRule" "sh: line 1: notACommand: command not found
minimake: *** command returned with non 0 code '127'.  Stop."
echo "No makefile or Makefile and no -f"
test_err "${program} test" "minimake: *** No rule to make target 'test'.  Stop."
test_err "${program}" "minimake: *** No targets specified and no makefile found.  Stop."
test_err "${program} -p test" "# variables
# rules"

echo "Parsing ---------------"
echo "Parsing - Syntax"
echo "Easy syntax"
test_stdout_file_print "parser/Makefile-syntax-test-easy"
echo "Hard syntax"
test_stdout_file_print "parser/Makefile-syntax-test-hard"
echo "Parsing - Expansion"
echo "Basic"
test_stdout_file_print "parser/Makefile-expansion-first"
echo "Recursive"
test_stdout_file_print "parser/Makefile-expansion-second"
echo "Complex"
test_stdout_file_print "parser/Makefile-expansion-tricky"
echo "Variable redefinition"
test_stdout_file_print "parser/Makefile-var-redefinition"
echo "Ignore empty target"
test_stdout_file_print "parser/Makefile-ignore-empty-target"
echo "Only tab lines (with and without comment)"
test_stdout_file_print "parser/Makefile-only-tab"
echo "Double dollar"
test_stdout_file "parser/Makefile-expansion-double-dollar"

echo "Building --------------"
echo "Target up to date"
touch "hello"
test_stdout "${program} -f testfiles/builder/Makefile-up-to-date hello" "minimake: 'hello' is up to date."
echo "Target without rule nothing to be done"
test_stdout "${program} -f testfiles/builder/Makefile-no-rule hello" "minimake: Nothing to be done for 'hello'."
rm "hello"
echo "Target nothing to be done"
test_stdout "${program} -f testfiles/builder/Makefile-nothing-to-be-done emptyrule" "minimake: Nothing to be done for 'emptyrule'."
echo "Target deduplication"
test_stdout "${program} -f testfiles/builder/Makefile-deduplication all all" "echo \"hello world\"
hello world
minimake: 'all' is up to date."

echo "Execution -------------"
echo "No logging"
test_stdout_file "execution/Makefile-logging-nologging"
echo "Recipe variable expansion easy"
test_stdout_file "execution/Makefile-recipe-expansion-simple"
echo "Recipe variable expansion hard"
test_stdout_file "execution/Makefile-recipe-expansion-hard"
echo "Special rule variables"
test_stdout_file "execution/Makefile-special-vars"
echo "Ignore empty target"
test_stdout_file "execution/Makefile-ignore-empty-target"
echo ".Phony target"
test_stdout "${program} -f testfiles/execution/Makefile-phony-single" "echo all
all"
test_stdout "${program} -f testfiles/execution/Makefile-phony-single all all" "echo all
all
minimake: Nothing to be done for 'all'."
test_stdout "${program} -f testfiles/execution/Makefile-phony-single test" "minimake: Nothing to be done for 'test'."
test_stdout "${program} -f testfiles/execution/Makefile-phony-first-multiple" "echo all
all"
echo "Multiple -f"
test_stdout "${program} -f testfiles/execution/Makefile-basic-1 -f testfiles/execution/Makefile-basic-2" "echo \"all 1\"
all 1
echo \"all 2\"
all 2"
echo "Rule -f"
test_stdout "${program} -f testfiles/execution/Makefile-basic-1 -f secret" "echo secret
secret"
echo "Implicit makefile"
mkdir tmp/
cd tmp/ || exit
touch makefile
touch Makefile
echo "MAKE=first" >> makefile
echo "MAKE=second" >> Makefile
test_stdout "${program} -p" "# variables
'MAKE' = 'first'
# rules"
echo "Implicit Makefile"
rm makefile
test_stdout "${program} -p" "# variables
'MAKE' = 'second'
# rules"
rm Makefile
cd ..
rmdir tmp/
echo "Pattern rules"
test_stdout_file "execution/Makefile-pattern-rules"
test_err "${program} -f double testfiles/execution-err/Makefile-pattern-rules" "minimake: *** No rule to make target 'double'.  Stop."

if [ ! "$1" = "SELFBUILTMINIMAKE" ]; then
    echo "Self Building ---------"
    mkdir selfbuild/
    cp ../Minimakefile selfbuild/Makefile
    cp -r ../src selfbuild/src
    cp -r ../tests selfbuild/tests
    cd selfbuild || exit
    ${program}
    ../testsuite.sh SELFBUILTMINIMAKE > /dev/null 2>&1
    #../testsuite.sh SELFBUILTMINIMAKE
    actual=$?
    expected=0
    print_test_result "${actual}" "${expected}"
    cd ..
    rm -rf selfbuild/
fi

echo "-----------------------"
echo "Total: ${total}"
echo "Passed: ${passed}"
echo "Failed: $((total-passed))"

if [ ${total} -eq ${passed} ]; then
    code=0
else
    code=1
fi

exit ${code}
