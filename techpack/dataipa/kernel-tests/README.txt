Subsystem: ipa-kernel-tests

Parameters:
  -n: Nominal test case (tests all the different use cases for ip_accelerator)
  -a: Adversarial test case (Currently holds no tests)
  -r: Repeatability test case (Currently holds no tests)
  -s: Stress test case (invokes many simultaneous threads that all try and access the device at once)
  --help: Specifies the params for run.sh

Description:
This test module tests IPA driver, it holds a userspace module and a kernel space module.