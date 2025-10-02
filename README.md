# Minimal GLFW metal-cpp cmake example

This example uses `metal-cpp` and a couple of additions to `metal-cpp-extensions` in order to show a pure C++ GLFW / Metal implementation.  It's really a port of [this](https://gist.github.com/gcatlin/987be74e2d58da96093a7598f3fbfb27) Objective-C based example.

The `metal-cpp-extensions` code was taken from this [Apple sample](https://developer.apple.com/metal/LearnMetalCPP.zip) and a couple of methods were added to `NS::Window` and `NS::View` to allow Metal to be set-up, as this code does not use `MetalKit`.

Note that Apple do not support `metal-cpp-extensions` and there are a couple of extensions that have been made to it, however it's pretty simple to extend yourself.
