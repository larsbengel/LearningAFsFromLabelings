import os
import time
import psutil
import argparse

import pandas as pd

from pathlib import Path

from subprocess import CalledProcessError, TimeoutExpired
from subprocess import Popen
from subprocess import TimeoutExpired
from subprocess import CalledProcessError
from subprocess import CompletedProcess

PIPE = -1
STDOUT = -2
DEVNULL = -3


def main():
    args = parse_cmd_input()
    output_file_dir = f"out/{args.name}/"
    results_file_name = f"results/{args.name}.csv"
    count = 0

    while os.path.exists(results_file_name):
        count += 1
        base_name, extension = os.path.splitext(results_file_name)
        results_file_name = base_name + f"_{count}" + extension

    if count > 1:
        output_file_dir += f"_{count}"

    Path(os.path.dirname(results_file_name)).mkdir(parents=True, exist_ok=True)
    #Path(results_file_name).touch()
    with open(results_file_name, 'w') as results_file:
        results_file.write("program,instance,labelings,format,timed_out,runtime,result,exit_with_error,error_code,error,cut_off,repetition\n")
    Path(output_file_dir).mkdir(parents=True, exist_ok=True)
    for repetition in range(args.repetitions):
        run_experiment(args.executable, args.directory, args.format, args.format, args.timeout, output_file_dir, results_file_name, repetition)

    return


def parse_cmd_input():
    parser = argparse.ArgumentParser(description="Tool for doing evaluations")
    parser.add_argument('-n', '--name', type=str, help="a name tzo identify the experiment")
    parser.add_argument('-d', '--directory', type=str, help="directory containing the dataset")
    parser.add_argument('-f', '--format', type=str, help="file extension to filter on")
    parser.add_argument('-e', '--executable', type=str, help="path to executable that should be evaluated")
    parser.add_argument('-t', '--timeout', type=int, help="timeout for each instance")
    parser.add_argument('-r', '--repetitions', type=int, help="number of repetitions for the dataset")
    return parser.parse_args()


def run_experiment(executable, directory, extension, format, timeout, output_file_dir, results_file_name, repetition):
    for file in os.scandir(directory):
        arguments_file = file.path
        if arguments_file.endswith(extension):
            labelings_file = arguments_file + ".L"
            results = run_solver(executable, arguments_file, labelings_file, format, timeout, output_file_dir, repetition)
            write_results_to_file(results, results_file_name)


def write_results_to_file(results, results_file):
    with open(results_file, 'a+') as file:
        file.write(f"{results['executable']},{results['instance']},{results['labelings']},{results['format']},{results['timed_out']},{results['runtime']},{results['result']},{results['exit_with_error']},{results['error_code']},{results['error']},{results['cut_off']},{results['repetition']}\n")
    

def run_solver(solver_path, arguments, labelings, format, timeout, output_file_dir=None, repetition=None):
        solver_dir = os.path.dirname(solver_path) + "/"
        instance_name = Path(arguments).stem

        results = {
            'executable': solver_path,
            'repetition': repetition,
            'instance': instance_name,
            'format':format,
            'labelings': labelings,
            'cut_off':timeout
        }

        params = ['bash', solver_path, 
                "-a", arguments,
                "-f", labelings,
                "-fo", format
                ]

        try:
            out_file_path = os.path.join(output_file_dir,f'{instance_name}_{repetition}.out')
            with open(out_file_path,'w') as output:
                start_time_current_run = time.perf_counter()
                if os.path.exists(solver_dir):
                    run_process(params, timeout=timeout, check=True,cwd=solver_dir,stdout=output)
                    end_time_current_run = time.perf_counter()
                    run_time = end_time_current_run - start_time_current_run
                    results.update({'timed_out':False,'runtime': run_time, 'result': out_file_path, 'exit_with_error': False, 'error_code': None,'error': None})
                else:
                    raise CalledProcessError(returncode=-1,cmd=params,output="Solver path not found.")

        except TimeoutExpired as e:
            results.update({'timed_out':True,'runtime': timeout, 'result': None, 'exit_with_error': False, 'error_code': None,'error': None})
        except CalledProcessError as err:
            print("\nError occured:",err)
            results.update({'timed_out':False,'runtime': None, 'result': None, 'exit_with_error': True, 'error_code': err.returncode,'error': err})

        return results


def run_process(*popenargs, input=None, capture_output=False, timeout=None, check=False, **kwargs):
    """Run command with arguments and return a CompletedProcess instance.

    The returned instance will have attributes args, returncode, stdout and
    stderr. By default, stdout and stderr are not captured, and those attributes
    will be None. Pass stdout=PIPE and/or stderr=PIPE in order to capture them.

    If check is True and the exit code was non-zero, it raises a
    CalledProcessError. The CalledProcessError object will have the return code
    in the returncode attribute, and output & stderr attributes if those streams
    were captured.

    If timeout is given, and the process takes too long, a TimeoutExpired
    exception will be raised.

    There is an optional argument "input", allowing you to
    pass bytes or a string to the subprocess's stdin.  If you use this argument
    you may not also use the Popen constructor's "stdin" argument, as
    it will be used internally.

    By default, all communication is in bytes, and therefore any "input" should
    be bytes, and the stdout and stderr will be bytes. If in text mode, any
    "input" should be a string, and stdout and stderr will be strings decoded
    according to locale encoding, or by "encoding" if set. Text mode is
    triggered by setting any of text, encoding, errors or universal_newlines.

    The other arguments are the same as for the Popen constructor.
    """
    if input is not None:
        if kwargs.get('stdin') is not None:
            raise ValueError('stdin and input arguments may not both be used.')
        kwargs['stdin'] = PIPE

    if capture_output:
        if kwargs.get('stdout') is not None or kwargs.get('stderr') is not None:
            raise ValueError('stdout and stderr arguments may not be used '
                             'with capture_output.')
        kwargs['stdout'] = PIPE
        kwargs['stderr'] = PIPE

    with Popen(*popenargs, **kwargs) as process:
        try:
            stdout, stderr = process.communicate(input, timeout=timeout)
        except TimeoutExpired as exc:
            parent = psutil.Process(process.pid)
            for child in parent.children(recursive=True):  # or parent.children() for recursive=False
                try:
                    child.kill()
                except psutil.NoSuchProcess:
                    continue
            parent.kill()
            process.kill()
            
            # POSIX _communicate already populated the output so
            # far into the TimeoutExpired exception.
            process.wait()

            raise
        except:  # Including KeyboardInterrupt, communicate handled that.
            print("PROBLEM2")
            process.kill()
            # We don't call process.wait() as .__exit__ does that for us.
            raise
        retcode = process.poll()
        if check and retcode:
            print("PROBLEM")
            raise CalledProcessError(retcode, process.args, output=stdout, stderr=stderr)
    return CompletedProcess(process.args, retcode, stdout, stderr)


if __name__ == "__main__":
    main()