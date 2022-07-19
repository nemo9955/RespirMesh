#!/usr/bin/env python3

class EasyDict(dict):
    """
    Dictionary subclass enabling attribute lookup/assignment of keys/values.

    For example::

        >>> m = EasyDict({'foo': 'bar'})
        >>> m.foo
        'bar'
        >>> m.has.foo
        True
        >>> m.has.not_here
        False
        >>> m.foo = 'not bar'
        >>> m['foo']
        'not bar'

    ``EasyDict`` objects also provide ``.first()`` which acts like
    ``.get()`` but accepts multiple keys as arguments, and returns the value of
    the first hit, e.g.::

        >>> m = EasyDict({'foo': 'bar', 'biz': 'baz'})
        >>> m.first('wrong', 'incorrect', 'foo', 'biz')
        'bar'

    """

    def __getattr__(self, key):
        if key == "has":
            return EasyDictValidator(self)

        try:
            return self[key]
        except KeyError:
            # to conform with __getattr__ spec
            raise AttributeError(key)

    def __setattr__(self, key, value):
        if key == "has":
            print(red('"has" is a reserved EasyDict word'))
            return
        self[key] = value

    def format(self, str_, *args):
        args_ = []

        for arg in args:
            # print( arg)
            args_.append(arg)
            # if isinstance(arg, (list, tuple)):
            #     args_.append(arg)
            # if isinstance(arg, dict):
            #     kwargs_.append(arg)
        return _RecursiveFormatterClass_().format(str_, *args_, **self)
