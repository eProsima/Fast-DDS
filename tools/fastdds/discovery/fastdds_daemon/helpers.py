# Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Modifications:
# - Adapted to work with Fast DDS CLI
#
# Original copyright retained below:
#
# Copyright 2021 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0

import functools
import inspect
import os
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
