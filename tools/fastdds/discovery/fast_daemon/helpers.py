from argparse import ArgumentTypeError
import functools
import inspect
import os
import sys
import time


def get_ros_domain_id():
    return int(os.environ.get('ROS_DOMAIN_ID', 0))


def wait_for(predicate, timeout, period=0.1):
    """
    Wait for a predicate to evaluate to `True`.

    :param timeout: duration, in seconds, to wait
      for the predicate to evaluate to `True`.
      Non-positive durations will result in an
      indefinite wait.
    :param period: predicate evaluation period,
      in seconds.
    :return: predicate result
    """
    if timeout < 0:
        timeout = float('+inf')
    deadline = time.time() + timeout
    while not predicate():
        if time.time() > deadline:
            return predicate()
        time.sleep(period)
    return True


def bind(func, *args, **kwargs):
    """
    Bind a function with a set of arguments.

    A functools.partial equivalent that is actually a function.
    """
    partial = functools.partial(func, *args, **kwargs)

    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        return partial(*args, **kwargs)
    wrapper.__signature__ = inspect.signature(func)
    return wrapper


def pretty_print_call(func, *args, **kwargs):
    """
    Print a function invocation.

    See `before_invocation` for usage as a hook.
    """
    name = func.__name__
    arguments = ', '.join(
        [f'{v!r}' for v in args] +
        [f'{k}={v!r}' for k, v in kwargs.items()]
    )
    print(f'{name}({arguments})')


def before_invocation(func, hook):
    """
    Invoke a `hook` before every `func` invocation.

    `hook` may take no arguments or take the `func`
    and arbitrary positional and keyword arguments.
    """
    signature = inspect.signature(hook)
    nargs = len(signature.parameters)
    if inspect.ismethod(hook):
        nargs = nargs - 1
    if nargs > 0:
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            hook(func, *args, **kwargs)
            return func(*args, **kwargs)
    else:
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            hook()
            return func(*args, **kwargs)
    wrapper.__signature__ = inspect.signature(func)
    return wrapper


def unsigned_int(string):
    try:
        value = int(string)
    except ValueError:
        value = -1
    if value < 0:
        raise ArgumentTypeError('value must be non-negative integer')
    return value


def collect_stdin():
    lines = b''
    while True:
        line = sys.stdin.buffer.readline()
        if not line:
            break
        lines += line
    return lines
