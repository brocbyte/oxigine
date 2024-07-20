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

typedef VkResult vkEnumerateInstanceExtensionPropertiesT(const char *, uint32_t *,
                                                         VkExtensionProperties *);
void OXIvkEnumerateInstanceExtensionProperties(
    vkEnumerateInstanceExtensionPropertiesT *vkEnumerateInstanceExtensionProperties,
    char *layerName) {

  uint32_t nInstanceExtensions;
  assert(vkEnumerateInstanceExtensionProperties(layerName, &nInstanceExtensions, 0) == VK_SUCCESS);
  VkExtensionProperties *pInstanceExtensions =
      (VkExtensionProperties *)malloc(nInstanceExtensions * sizeof(VkExtensionProperties));
  assert(vkEnumerateInstanceExtensionProperties(layerName, &nInstanceExtensions,
                                                pInstanceExtensions) == VK_SUCCESS);
  fprintf(logFile, "\t\textensions: {\n");
  for (int i = 0; i < nInstanceExtensions; ++i) {
    fprintf(logFile, "\t\t\t%s,\n", pInstanceExtensions[i].extensionName);
  }
  fprintf(logFile, "\t\t}\n");
  free(pInstanceExtensions);
}

static VKAPI_ATTR VkBool32 callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                    void *pUserData) {
  fprintf(logFile, "callback: %s\n", pCallbackData->pMessage);
  return VK_FALSE;
}

/**
 * https://github.com/KhronosGroup/Vulkan-Loader/blob/main/docs/LoaderInterfaceArchitecture.md
 * https://github.com/KhronosGroup/Vulkan-Loader/blob/main/docs/LoaderApplicationInterface.md
 */
static int win32LoadVulkan() {
  /* Indirectly Linking to the Loader */
  HMODULE vulkan = LoadLibraryW(L"vulkan-1.dll");
  if (!vulkan) {
    fprintf(logFile, "No vulkan!\n");
    return -1;
  }

  typedef PFN_vkVoidFunction vkGetInstanceProcAddrT(VkInstance, const char *);
  vkGetInstanceProcAddrT *vkGetInstanceProcAddr =
      (vkGetInstanceProcAddrT *)(GetProcAddress(vulkan, "vkGetInstanceProcAddr"));

  if (!vkGetInstanceProcAddr) {
    fprintf(logFile, "No vkGetInstanceProcAddr!\n");
    return -1;
  }

  typedef VkResult vkEnumerateInstanceLayerPropertiesT(uint32_t *, VkLayerProperties *);
  vkEnumerateInstanceLayerPropertiesT *vkEnumerateInstanceLayerProperties =
      (vkEnumerateInstanceLayerPropertiesT *)(vkGetInstanceProcAddr(
          0, "vkEnumerateInstanceLayerProperties"));
  if (!vkEnumerateInstanceLayerProperties) {
    fprintf(logFile, "No vkEnumerateInstanceLayerProperties!\n");
    return -1;
  }

  vkEnumerateInstanceExtensionPropertiesT *vkEnumerateInstanceExtensionProperties =
      (vkEnumerateInstanceExtensionPropertiesT *)(vkGetInstanceProcAddr(
          0, "vkEnumerateInstanceExtensionProperties"));
  if (!vkEnumerateInstanceExtensionProperties) {
    fprintf(logFile, "No vkEnumerateInstanceExtensionProperties!\n");
    return -1;
  }

  typedef VkResult vkCreateInstanceT(const VkInstanceCreateInfo *, const VkAllocationCallbacks *,
                                     VkInstance *);
  vkCreateInstanceT *vkCreateInstance =
      (vkCreateInstanceT *)(vkGetInstanceProcAddr(0, "vkCreateInstance"));
  if (!vkCreateInstance) {
    fprintf(logFile, "No vkCreateInstance!\n");
    return -1;
  }

  OXIvkEnumerateInstanceExtensionProperties(vkEnumerateInstanceExtensionProperties, 0);
  uint32_t nInstanceLayers;
  assert(vkEnumerateInstanceLayerProperties(&nInstanceLayers, 0) == VK_SUCCESS);
  VkLayerProperties *pInstanceLayers =
      (VkLayerProperties *)malloc(nInstanceLayers * sizeof(VkLayerProperties));
  assert(vkEnumerateInstanceLayerProperties(&nInstanceLayers, pInstanceLayers) == VK_SUCCESS);
  fprintf(logFile, "Layers available: {\n");
  for (int i = 0; i < nInstanceLayers; ++i) {
    fprintf(logFile, "\t%s:{\n", pInstanceLayers[i].layerName);
    OXIvkEnumerateInstanceExtensionProperties(vkEnumerateInstanceExtensionProperties,
                                              pInstanceLayers[i].layerName);
    fprintf(logFile, "\t},\n");
  }
  fprintf(logFile, "},\n");
  free(pInstanceLayers);

  char *instance_layers[] = {"VK_LAYER_KHRONOS_validation"};
  char *instance_extensions[] = {"VK_EXT_debug_utils"};
  const VkApplicationInfo app_info = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                      .pApplicationName = "OXIGINE_APP",
                                      .pEngineName = "OXIGINE_ENGINE",
                                      .apiVersion = VK_API_VERSION_1_0};
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
      callback};
  VkValidationFeatureEnableEXT enabled_features[] = {
      VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
      VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
      VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT};
  const VkValidationFeaturesEXT validation_features = {
      .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
      .enabledValidationFeatureCount = sizeof(enabled_features) / sizeof(enabled_features[0]),
      .pEnabledValidationFeatures = enabled_features,
      .pNext = &debugUtilsMessengerCreateInfoEXT};

  const VkInstanceCreateInfo instance_info = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                              .pNext = &validation_features,
                                              .pApplicationInfo = &app_info,
                                              .enabledLayerCount = 1,
                                              .ppEnabledLayerNames = instance_layers,
                                              .enabledExtensionCount = 1,
                                              .ppEnabledExtensionNames = instance_extensions};
  VkInstance instance;
  assert(vkCreateInstance(&instance_info, 0, &instance) == VK_SUCCESS);

  PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
      (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
                                                                "vkCreateDebugUtilsMessengerEXT");
  if (!vkCreateDebugUtilsMessengerEXT) {
    fprintf(logFile, "((\n");
    return -1;
  }

  VkDebugUtilsMessengerEXT messenger;
  assert(vkCreateDebugUtilsMessengerEXT(instance, &debugUtilsMessengerCreateInfoEXT, 0,
                                        &messenger) == VK_SUCCESS);
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