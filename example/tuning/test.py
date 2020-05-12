import os
import sys
import tempfile
import subprocess


def run_command(command):
    p = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
    while True:
        line = p.stdout.readline()
        if not line:
            break
        print line,
    sys.stdout.flush()


def test_configuration(xml_grammar, instance, depth, max_steps, seed, parameters):
    base_path = os.path.realpath(sys.path[0] + '/..')
    os.chdir(base_path)

    # create temp directory
    directory_name = tempfile.mkdtemp()
    os.system('cp -r ' + base_path + '/sources/* ' + directory_name)

    # generate the code
    print directory_name
    command = base_path + '/build/grammar2code -d ' + depth + ' -g ' + xml_grammar + ' -t ' + directory_name + ' ' + ' '.join(parameters)
    run_command(command)

    # testing the code
    os.chdir(directory_name)
    os.system('make')
    command = './auto_algo ' + instance + ' ' + max_steps + ' ' + seed
    run_command(command)

    os.system('rm -rf ' + directory_name)

if __name__ == '__main__':
    test_configuration(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], sys.argv[6:])
