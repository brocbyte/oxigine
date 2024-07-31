#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <stdbool.h>
#include <stdint.h>
#include <windows.h>

#include <assert.h>
#include <stdio.h>
#define VK_USE_PLATFORM_WIN32_KHR 1
#include <vulkan/vulkan.h>

#define OXILog(...)                                                                                \
  do {                                                                                             \
    fprintf(logFile, __VA_ARGS__);                                                                 \
  } while (false)

#define OXIAssertT(exp, ...)                                                                       \
  do {                                                                                             \
    if (!(exp)) {                                                                                  \
      OXILog(__VA_ARGS__);                                                                         \
      exit(-1);                                                                                    \
    }                                                                                              \
  } while (false)

#define OXIAssert(exp) OXIAssertT(exp, "%s:%d", __FILE__, __LINE__)
#define VOK(val) OXIAssert(val == VK_SUCCESS)

static struct {
  // instance functions
  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
  PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;

  PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
  PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
  PFN_vkCreateInstance vkCreateInstance;
  PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
  PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
  PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
  PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
  PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
  PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
  PFN_vkCreateDevice vkCreateDevice;

  //  platform instance functions
  PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
  PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;

  // device functions
  PFN_vkGetDeviceQueue vkGetDeviceQueue;
  PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
  PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
} vt;

#define VK_GET_INSTANCE_PROC_ADDR(instance, name)                                                  \
  do {                                                                                             \
    vt.name = (PFN_##name)(vt.vkGetInstanceProcAddr(instance, #name));                             \
    OXIAssert(vt.name);                                                                            \
  } while (false)

#define VK_GET_DEVICE_PROC_ADDR(device, name)                                                      \
  do {                                                                                             \
    vt.name = (PFN_##name)(vt.vkGetDeviceProcAddr(device, #name));                                 \
    OXIAssert(vt.name);                                                                            \
  } while (false)

#define asize(arr) ((sizeof(arr) / sizeof(arr[0])))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef i32 b32;
typedef float f32;
typedef double f64;

static b32 globalRunning;
static FILE *logFile;

LRESULT CALLBACK win32WindowsCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  LRESULT result = 0;
  switch (uMsg) {
  case WM_CREATE: {
  } break;
  case WM_SIZE: {
  } break;
  case WM_DESTROY: {
    globalRunning = false;
  } break;
  case WM_CLOSE: {
    globalRunning = false;
  } break;
  case WM_ACTIVATEAPP: {
  } break;
  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
  case WM_KEYDOWN:
  case WM_KEYUP: {
  } break;
  case WM_PAINT: {
    PAINTSTRUCT paint;
    HDC hdc = BeginPaint(hwnd, &paint);
    EndPaint(hwnd, &paint);
  } break;
  default: {
    result = DefWindowProc(hwnd, uMsg, wParam, lParam);
  } break;
  }
  return result;
}

static void win32ProcessMessages() {
  MSG message;
  while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
    switch (message.message) {
    case WM_QUIT: {
      globalRunning = false;
    } break;
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    default: {
      TranslateMessage(&message);
      DispatchMessage(&message);
    }
    }
  }
}

#define DEFINE_DARRAY0(Type, Func, ...)                                                            \
  u32 n##Type;                                                                                     \
  VOK(vt.##Func(&n##Type, 0));                                                                     \
  Type *p##Type = (Type *)malloc(n##Type * sizeof(Type));                                          \
  VOK(vt.##Func(&n##Type, p##Type))

#define DEFINE_DARRAY(Type, Func, Check, ...)                                                      \
  u32 n##Type;                                                                                     \
  Check(vt.##Func(__VA_ARGS__, &n##Type, 0));                                                      \
  Type *p##Type = (Type *)malloc(n##Type * sizeof(Type));                                          \
  Check(vt.##Func(__VA_ARGS__, &n##Type, p##Type))

static void OXIvkEnumerateInstanceExtensionProperties(char *layerName) {
  DEFINE_DARRAY(VkExtensionProperties, vkEnumerateInstanceExtensionProperties, VOK, layerName);
  OXILog("\t\textensions: {\n");
  for (int i = 0; i < nVkExtensionProperties; ++i) {
    OXILog("\t\t\t%s,\n", pVkExtensionProperties[i].extensionName);
  }
  OXILog("\t\t}\n");
  free(pVkExtensionProperties);
}

static void OXIvkEnumerateInstanceLayerProperties() {
  DEFINE_DARRAY0(VkLayerProperties, vkEnumerateInstanceLayerProperties);
  OXILog("Layers available: {\n");
  for (int i = 0; i < nVkLayerProperties; ++i) {
    OXILog("\t%s:{\n", pVkLayerProperties[i].layerName);
    OXIvkEnumerateInstanceExtensionProperties(pVkLayerProperties[i].layerName);
    OXILog("\t},\n");
  }
  OXILog("},\n");
  free(pVkLayerProperties);
}

static VKAPI_ATTR VkBool32
debugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                       VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
  OXILog("debug: %s\n", pCallbackData->pMessage);
  return VK_FALSE;
}

/**
 * https://github.com/KhronosGroup/Vulkan-Loader/blob/main/docs/LoaderInterfaceArchitecture.md
 * https://github.com/KhronosGroup/Vulkan-Loader/blob/main/docs/LoaderApplicationInterface.md
 */
static void win32LoadVulkan() {
  /* Indirectly Linking to the Loader */
  HMODULE vulkan = LoadLibraryW(L"vulkan-1.dll");
  OXIAssert(vulkan);

  vt.vkGetInstanceProcAddr =
      (PFN_vkGetInstanceProcAddr)(GetProcAddress(vulkan, "vkGetInstanceProcAddr"));

  OXIAssert(vt.vkGetInstanceProcAddr);
}

static void dbgEnumerateInstanceStuff() {
  VK_GET_INSTANCE_PROC_ADDR(0, vkEnumerateInstanceLayerProperties);
  VK_GET_INSTANCE_PROC_ADDR(0, vkEnumerateInstanceExtensionProperties);

  OXIvkEnumerateInstanceExtensionProperties(0);
  OXIvkEnumerateInstanceLayerProperties();
}

#define MESSAGE_SEVERITY(F) VK_DEBUG_UTILS_MESSAGE_SEVERITY_##F##_BIT_EXT
#define MESSAGE_TYPE(F) VK_DEBUG_UTILS_MESSAGE_TYPE_##F##_BIT_EXT
#define VALIDATION_FEATURE(F) VK_VALIDATION_FEATURE_ENABLE_##F##_EXT
static VkInstance OXIvkCreateInstance() {
  VK_GET_INSTANCE_PROC_ADDR(0, vkCreateInstance);

  VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = MESSAGE_SEVERITY(VERBOSE) | MESSAGE_SEVERITY(WARNING) |
                         MESSAGE_SEVERITY(INFO) | MESSAGE_SEVERITY(ERROR),
      .messageType = MESSAGE_TYPE(GENERAL) | MESSAGE_TYPE(VALIDATION) | MESSAGE_TYPE(PERFORMANCE) |
                     MESSAGE_TYPE(DEVICE_ADDRESS_BINDING),
      debugMessengerCallback};

  VkValidationFeatureEnableEXT aValidationFeatureEnableEXT[] = {
      VALIDATION_FEATURE(BEST_PRACTICES), VALIDATION_FEATURE(DEBUG_PRINTF),
      VALIDATION_FEATURE(SYNCHRONIZATION_VALIDATION)};

  const VkValidationFeaturesEXT validationFeaturesEXT = {
      .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
      .enabledValidationFeatureCount = asize(aValidationFeatureEnableEXT),
      .pEnabledValidationFeatures = aValidationFeatureEnableEXT,
      .pNext = &debugUtilsMessengerCreateInfoEXT};

  char *instance_layers[] = {"VK_LAYER_KHRONOS_validation"};
  char *instance_extensions[] = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME, "VK_KHR_surface",
                                 "VK_KHR_win32_surface"};
  const VkApplicationInfo applicationInfo = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                             .pApplicationName = "OXIGINE_APP",
                                             .pEngineName = "OXIGINE_ENGINE",
                                             .apiVersion = VK_API_VERSION_1_3};
  const VkInstanceCreateInfo instanceCreateInfo = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                                   .pNext = &validationFeaturesEXT,
                                                   .pApplicationInfo = &applicationInfo,
                                                   .enabledLayerCount = 1,
                                                   .ppEnabledLayerNames = instance_layers,
                                                   .enabledExtensionCount = 3,
                                                   .ppEnabledExtensionNames = instance_extensions};
  VkInstance instance;
  VOK(vt.vkCreateInstance(&instanceCreateInfo, 0, &instance));

  VK_GET_INSTANCE_PROC_ADDR(instance, vkCreateDebugUtilsMessengerEXT);

  VkDebugUtilsMessengerEXT messenger;
  VOK(vt.vkCreateDebugUtilsMessengerEXT(instance, &debugUtilsMessengerCreateInfoEXT, 0,
                                        &messenger));
  return instance;
}
#undef VALIDATION_FEATURE
#undef MESSAGE_TYPE
#undef MESSAGE_SEVERITY

typedef struct OXIVkPhysicalDevice {
  VkPhysicalDevice vkPhysicalDevice;
  u32 graphicsQueueFamilyIdx;
} OXIVkPhysicalDevice;

OXIVkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
  VK_GET_INSTANCE_PROC_ADDR(instance, vkEnumeratePhysicalDevices);
  VK_GET_INSTANCE_PROC_ADDR(instance, vkGetPhysicalDeviceProperties);
  VK_GET_INSTANCE_PROC_ADDR(instance, vkGetPhysicalDeviceFeatures);
  VK_GET_INSTANCE_PROC_ADDR(instance, vkGetPhysicalDeviceQueueFamilyProperties);
  VK_GET_INSTANCE_PROC_ADDR(instance, vkGetPhysicalDeviceSurfaceSupportKHR);

  OXIVkPhysicalDevice result = {.vkPhysicalDevice = VK_NULL_HANDLE, .graphicsQueueFamilyIdx = -1};
  DEFINE_DARRAY(VkPhysicalDevice, vkEnumeratePhysicalDevices, VOK, instance);
  OXILog("nPhysicalDevices: %d\n", nVkPhysicalDevice);

  for (int i = 0; i < nVkPhysicalDevice; ++i) {
    VkPhysicalDeviceProperties deviceProperties;
    vt.vkGetPhysicalDeviceProperties(pVkPhysicalDevice[i], &deviceProperties);
    VkPhysicalDeviceFeatures deviceFeatures;
    vt.vkGetPhysicalDeviceFeatures(pVkPhysicalDevice[i], &deviceFeatures);
    OXILog("deviceName: %s\n", deviceProperties.deviceName);
    if ((deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) &&
        deviceFeatures.geometryShader) {
      result.vkPhysicalDevice = pVkPhysicalDevice[i];
    }
  }
  free(pVkPhysicalDevice);

  DEFINE_DARRAY(VkQueueFamilyProperties, vkGetPhysicalDeviceQueueFamilyProperties, (void),
                result.vkPhysicalDevice);
  for (int i = 0; i < nVkQueueFamilyProperties; ++i) {
    VkBool32 surfaceSupport;
    vt.vkGetPhysicalDeviceSurfaceSupportKHR(result.vkPhysicalDevice, i, surface, &surfaceSupport);
    bool graphicsSupport = (pVkQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
    if (graphicsSupport && (surfaceSupport == VK_TRUE)) {
      result.graphicsQueueFamilyIdx = i;
    }
  }
  free(pVkQueueFamilyProperties);
  return result;
}

VkDevice OXIvkCreateDevice(VkInstance instance, OXIVkPhysicalDevice *physicalDevice) {
  VK_GET_INSTANCE_PROC_ADDR(instance, vkCreateDevice);
  VkDevice result;
  float queuePriority = 1.0f;
  VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = physicalDevice->graphicsQueueFamilyIdx,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority};
  VkPhysicalDeviceFeatures deviceFeatures = {0};
  char *extension_names[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  const VkDeviceCreateInfo deviceCreateInfo = {.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                               .queueCreateInfoCount = 1,
                                               .pQueueCreateInfos = &deviceQueueCreateInfo,
                                               .pEnabledFeatures = &deviceFeatures,
                                               .ppEnabledExtensionNames = extension_names,
                                               .enabledExtensionCount = 1};
  VOK(vt.vkCreateDevice(physicalDevice->vkPhysicalDevice, &deviceCreateInfo, 0, &result));
  return result;
}

VkQueue OXIvkGetDeviceQueue(VkDevice device, OXIVkPhysicalDevice *physicalDevice) {
  VK_GET_DEVICE_PROC_ADDR(device, vkGetDeviceQueue);
  VkQueue queue;
  vt.vkGetDeviceQueue(device, physicalDevice->graphicsQueueFamilyIdx, 0, &queue);
  return queue;
}

VkSurfaceKHR OXIvkCreateWin32SurfaceKHR(VkInstance instance, HINSTANCE hInstance, HWND hwnd) {
  VK_GET_INSTANCE_PROC_ADDR(instance, vkCreateWin32SurfaceKHR);
  VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfoKHR = {
      .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
      .hwnd = hwnd,
      .hinstance = hInstance};
  VkSurfaceKHR surface;
  VOK(vt.vkCreateWin32SurfaceKHR(instance, &win32SurfaceCreateInfoKHR, 0, &surface));
  return surface;
}

typedef struct OXIVkSwapchainKHR {
  VkSwapchainKHR vkSwapchainKHR;
  VkImage *pSwapchainImages;
  u32 nSwapchainImages;
} OXIVkSwapchainKHR;

OXIVkSwapchainKHR OXIvkCreateSwapchainKHR(VkInstance instance, VkDevice device,
                                          VkSurfaceKHR surface, OXIVkPhysicalDevice *physicalDevice,
                                          RECT *rect) {
  OXIVkSwapchainKHR result;
  VK_GET_INSTANCE_PROC_ADDR(instance, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
  VK_GET_DEVICE_PROC_ADDR(device, vkCreateSwapchainKHR);
  VkSurfaceCapabilitiesKHR surfaceCapabilitiesKHR;
  VOK(vt.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice->vkPhysicalDevice, surface,
                                                   &surfaceCapabilitiesKHR));
  VkExtent2D swapchainExtent = {0};
  if (surfaceCapabilitiesKHR.currentExtent.width == -1 ||
      surfaceCapabilitiesKHR.currentExtent.height == -1) {
    swapchainExtent.width = rect->right - rect->left;
    swapchainExtent.height = rect->top - rect->bottom;
  } else {
    swapchainExtent = surfaceCapabilitiesKHR.currentExtent;
  }
  VkSwapchainCreateInfoKHR createInfo = {.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                                         .surface = surface,
                                         .minImageCount = surfaceCapabilitiesKHR.minImageCount,
                                         .imageFormat = VK_FORMAT_B8G8R8A8_SRGB,
                                         .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
                                         .imageExtent = swapchainExtent,
                                         .imageArrayLayers = 1,
                                         .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                         .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                         .presentMode = VK_PRESENT_MODE_MAILBOX_KHR,
                                         .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
                                         .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR};
  VOK(vt.vkCreateSwapchainKHR(device, &createInfo, 0, &result.vkSwapchainKHR));

  VK_GET_DEVICE_PROC_ADDR(device, vkGetSwapchainImagesKHR);
  DEFINE_DARRAY(VkImage, vkGetSwapchainImagesKHR, VOK, device, result.vkSwapchainKHR);
  result.nSwapchainImages = nVkImage;
  result.pSwapchainImages = pVkImage;
  return result;
}

void win32SetupRenderer(HINSTANCE hInstance, HWND hwnd) {
  win32LoadVulkan();
#ifdef OXIDEBUG
  dbgEnumerateInstanceStuff();
#endif
  VkInstance instance = OXIvkCreateInstance();
  VkSurfaceKHR surface = OXIvkCreateWin32SurfaceKHR(instance, hInstance, hwnd);

  OXIVkPhysicalDevice physicalDevice = pickPhysicalDevice(instance, surface);
  VkDevice device = OXIvkCreateDevice(instance, &physicalDevice);

  VK_GET_INSTANCE_PROC_ADDR(instance, vkGetDeviceProcAddr);

  VkQueue queue = OXIvkGetDeviceQueue(device, &physicalDevice);

  RECT windowRect;
  OXIAssert(GetWindowRect(hwnd, &windowRect));
  OXIVkSwapchainKHR swapchain =
      OXIvkCreateSwapchainKHR(instance, device, surface, &physicalDevice, &windowRect);
}

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
  WNDCLASS WindowClass = {.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
                          .lpfnWndProc = win32WindowsCallback,
                          .hInstance = hInstance,
                          //.hIcon =
                          .lpszClassName = L"OxigineWindowClass"};
  if (!RegisterClass(&WindowClass))
    exit(-1);
  HWND window = CreateWindowEx(0 /*WS_EX_TOPMOST | WS_EX_LAYERED*/, WindowClass.lpszClassName,
                               L"Oxigine", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                               CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, 0);
  if (!window)
    exit(-1);

  logFile = fopen("last_run.log", "w");
  win32SetupRenderer(hInstance, window);

  globalRunning = true;
  // while (globalRunning) {
  //   win32ProcessMessages();
  // }

  fclose(logFile);
}