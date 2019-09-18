gRPC Image Rotation Service
===========================

Description
-----------

You've been given a protobuf definition file that defines an image rotation and "something else useful"* [gRPC](https://grpc.io/) interface.  Your task is to create a server that implements this interface.  Time permitting, implement the server as if it will operate in a production environment and be subsequently used and worked on by a team of engineers.  If you make compromises to keep your solution simple and timely, please be sure to bring them up in the discussion part of your deliverables.

We will run and test your solution on inputs of varying formats and integrity, so be sure to include instructions on how to get it up and running. Your instructions should assume we'll start with a fresh Ubuntu 18.04 or Mac OS install (please let us know which one).

You can use any officially supported gRPC language (see: https://grpc.io/docs/).

\* For the "something else useful" endpoint, you can choose anything you want that takes an image as input and returns some interesting and useful result.  The result can be an image, a number, a vector, or anything really.  This is your chance to show us your ability to implement algorithms, be pragmatic, creative, or anything.  It's up to you.

Deliverables
------------

There are 4 deliverables that should come in the form of this git repository (either zipped and sent to us, or made available as a `clone`-able repository):

1. A `gRPC` server that implements the `NLImageService` interface
2. Instructions on how to test your solution given a fresh Ubuntu 18.04 installation or fresh Mac OS (please specify).
3. A simple CLI test client we can use to test your solution on a variety of images of our choosing.
4. Discussion of limitations or known issues with your solution, how you'd change it for production given more time and resources, and any other thoughts you have about the problem, your solution, or the tools you used.



