.. title:: clang-tidy - hsc-unused-return-value

hsc-unused-return-value
=======================

Finds discarded return values from functions marked
``[[nodiscard]]`` or ``__attribute__((warn_unused_result))``.

Examples
--------

.. code-block:: c++

  int mustUse() __attribute__((warn_unused_result));

  void f() {
    mustUse();
  }

The check warns because the result of ``mustUse`` is ignored.

The following is accepted:

.. code-block:: c++

  void f() {
    (void)mustUse();
  }
