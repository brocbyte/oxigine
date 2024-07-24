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
#include <vulkan/vulkan.h>

#define OXIAssertT(exp, ...)                                                                       \
  do {                                                                                             \
    if (!(exp)) {                                                                                  \
      fprintf(logFile, __VA_ARGS__);                                                               \
      exit(-1);                                                                                    \
    }                                                                                              \
  } while (false)

#define OXIAssert(exp) OXIAssertT(exp, "%s:%d", __FILE__, __LINE__)
#define VOK(val) OXIAssert(val == VK_SUCCESS)

static struct {
  PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
  PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
  PFN_vkCreateInstance vkCreateInstance;
  PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
} vt;

#define VK_GET_INSTANCE_PROC_ADDR(instance, name)                                                  \
  do {                                                                                             \
    vt.name = (PFN_##name)(vkGetInstanceProcAddr(instance, #name));                                \
    OXIAssert(vt.name);                                                                           \
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

static void OXIvkEnumerateInstanceExtensionProperties(char *layerName) {
  uint32_t nExtensions;
  VOK(vt.vkEnumerateInstanceExtensionProperties(layerName, &nExtensions, 0));

  VkExtensionProperties *pExtensions =
      (VkExtensionProperties *)malloc(nExtensions * sizeof(VkExtensionProperties));
  VOK(vt.vkEnumerateInstanceExtensionProperties(layerName, &nExtensions, pExtensions));
  fprintf(logFile, "\t\textensions: {\n");
  for (int i = 0; i < nExtensions; ++i) {
    fprintf(logFile, "\t\t\t%s,\n", pExtensions[i].extensionName);
  }
  fprintf(logFile, "\t\t}\n");
  free(pExtensions);
}

static void OXIvkEnumerateInstanceLayerProperties() {
  uint32_t nLayers;
  VOK(vt.vkEnumerateInstanceLayerProperties(&nLayers, 0));
  VkLayerProperties *pLayers = (VkLayerProperties *)malloc(nLayers * sizeof(VkLayerProperties));
  VOK(vt.vkEnumerateInstanceLayerProperties(&nLayers, pLayers));
  fprintf(logFile, "Layers available: {\n");
  for (int i = 0; i < nLayers; ++i) {
    fprintf(logFile, "\t%s:{\n", pLayers[i].layerName);
    OXIvkEnumerateInstanceExtensionProperties(pLayers[i].layerName);
    fprintf(logFile, "\t},\n");
  }
  fprintf(logFile, "},\n");
  free(pLayers);
}

static VKAPI_ATTR VkBool32
debugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                       VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
  fprintf(logFile, "debug: %s\n", pCallbackData->pMessage);
  return VK_FALSE;
}

/**
 * https://github.com/KhronosGroup/Vulkan-Loader/blob/main/docs/LoaderInterfaceArchitecture.md
 * https://github.com/KhronosGroup/Vulkan-Loader/blob/main/docs/LoaderApplicationInterface.md
 */
static int win32LoadVulkan() {
  /* Indirectly Linking to the Loader */
  HMODULE vulkan = LoadLibraryW(L"vulkan-1.dll");
  OXIAssert(vulkan);

  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
      (PFN_vkGetInstanceProcAddr)(GetProcAddress(vulkan, "vkGetInstanceProcAddr"));

  OXIAssert(vkGetInstanceProcAddr);

#ifdef OXIDEBUG
  // debug: enumerate layers & extensions
  VK_GET_INSTANCE_PROC_ADDR(0, vkEnumerateInstanceLayerProperties);
  VK_GET_INSTANCE_PROC_ADDR(0, vkEnumerateInstanceExtensionProperties);

  OXIvkEnumerateInstanceExtensionProperties(0);
  OXIvkEnumerateInstanceLayerProperties();
#endif

  VK_GET_INSTANCE_PROC_ADDR(0, vkCreateInstance);
  VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
      debugMessengerCallback};
  VkValidationFeatureEnableEXT aValidationFeatureEnableEXT[] = {
      VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
      VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
      VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT};
  const VkValidationFeaturesEXT validationFeaturesEXT = {
      .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
      .enabledValidationFeatureCount = asize(aValidationFeatureEnableEXT),
      .pEnabledValidationFeatures = aValidationFeatureEnableEXT,
      .pNext = &debugUtilsMessengerCreateInfoEXT};

  char *instance_layers[] = {"VK_LAYER_KHRONOS_validation"};
  char *instance_extensions[] = {"VK_EXT_debug_utils"};
  const VkApplicationInfo applicationInfo = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                             .pApplicationName = "OXIGINE_APP",
                                             .pEngineName = "OXIGINE_ENGINE",
                                             .apiVersion = VK_API_VERSION_1_3};
  const VkInstanceCreateInfo instanceCreateInfo = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                                   .pNext = &validationFeaturesEXT,
                                                   .pApplicationInfo = &applicationInfo,
                                                   .enabledLayerCount = 1,
                                                   .ppEnabledLayerNames = instance_layers,
                                                   .enabledExtensionCount = 1,
                                                   .ppEnabledExtensionNames = instance_extensions};
  VkInstance instance;
  VOK(vt.vkCreateInstance(&instanceCreateInfo, 0, &instance));

  VK_GET_INSTANCE_PROC_ADDR(instance, vkCreateDebugUtilsMessengerEXT);

  VkDebugUtilsMessengerEXT messenger;
  VOK(vt.vkCreateDebugUtilsMessengerEXT(instance, &debugUtilsMessengerCreateInfoEXT, 0,
                                        &messenger));
  // VkPhysicalDevice
  // VkDevice
  return 0;
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

  if (win32LoadVulkan() != 0) {
    exit(-1);
  }

  globalRunning = true;
  // while (globalRunning) {
  //   win32ProcessMessages();
  // }

  fclose(logFile);
}