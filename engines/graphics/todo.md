# TODO
- [ ] cleanup Engine::createInstance() code
- [ ] cleanup Engine::pickPhysicalDevice() code
- [ ] cleanup Engine::createLogicalDevice() code
- [ ] Engine::findQueueFamilies() not used? contained in pickPhysicalDevice
- [ ] Engine::isDeviceSuitable() not used? contained in pickPhysicalDevice
- [ ] cleanup everything
- [ ] turn swapchain into a class
- [ ] turn other components (X from createX() methods) into classes?

# DONE
- [x] make it work with two different queues for graphics and presentation (on Engine::createLogicalDevice())
