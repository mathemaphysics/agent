Agent
==============================

.. toctree::
   :maxdepth: 2
   :caption: Contents:

Indices and tables
==================
* :ref:`genindex`
* :ref:`search`

Welcome to the documentation for agent, a worker thread model code intended to
be simple and easy to use. Everything you'll see here makes use of the ``IWorker``
abstract class to form workers and define how they interact with one another. It
also contains additional classes which interface directly with the AMQP protocol
for use with RabbitMQ and related implementations.

.. doxygenclass:: agent::IWorker