import subprocess

if __name__ == '__main__':
   
    successes_0 = 0
    failures_0 = 0 
    iterations = 500

    for i in range(iterations):
        exit_code = subprocess.call(['./readWriteTest/threads'])
        if (exit_code == 0):
            successes_0 = successes_0 + 1
        else:
            failures_0 = failures_0 +1

  
    successes = 0
    failures = 0 
 
    for i in range(iterations):
        exit_code = subprocess.call(['./openTest/threads'])
        if (exit_code == 0):
            successes = successes + 1
        else:
            failures = failures +1

    print(f'\033[1;32m Test 1... Passed: {successes_0}/{iterations}\t \033[1;31mFailed:{failures_0}/{iterations}\033[0m')
    print(f'\033[1;32m Test 2... Passed: {successes}/{iterations}\t \033[1;31mFailed:{failures}/{iterations}\033[0m')
    subprocess.call(['./big'])
 
