import subprocess

if __name__ == '__main__':
    
    successes = 0
    failures = 0
    iterations = 500

    for i in range(iterations):
        exit_code = subprocess.call(['./threads'])
        if (exit_code == 0):
            successes = successes + 1
        else:
            failures = failures +1

    print(f'\033[1;32mPassed: {successes}/{iterations}\t \033[1;31mFailed:{failures}/{iterations}\033[0m')
    
