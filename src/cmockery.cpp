//
// Created by oslab on 23-3-16.
//
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <vector>
#include <iostream>
#include <csetjmp>
#include <csignal>
#include <cstring>
#include <sys/time.h>
#include "../include/cmockery.h"

// 动态分配块周围的保护字节大小。
#define MALLOC_GUARD_SIZE 16
// Pattern used to initialize guard blocks.用于初始化保护块的模式。
#define MALLOC_GUARD_PATTERN 0xEF
/* Pattern used to initialize memory allocated with test_malloc().
 * 用于初始化使用 test_malloc（） 分配的内存的模式。
 */
#define MALLOC_ALLOC_PATTERN 0xBA
#define MALLOC_FREE_PATTERN 0xCD
// Alignment of allocated blocks.对齐分配的块  NOTE: This must be base2.必须是2的倍数
#define MALLOC_ALIGNMENT sizeof(size_t) //64位中为8,32位中为4

// Printf formatting for source code locations.
#define SOURCE_LOCATION_FORMAT "%s:%d"

#ifndef PRIxMAX
#define PRIxMAX "llx"
#endif
// 计算数组中的元素数
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

// Declare and initialize the pointer member of ValuePointer variable name
// with ptr.
#define declare_initialize_value_pointer_pointer(name, ptr) \
    ValuePointer name ; \
    name.value = 0; \
    name.pointer = (void*)(ptr)

// Declare and initialize the value member of ValuePointer variable name
// with val.
#define declare_initialize_value_pointer_value(name, val) \
    ValuePointer name ; \
    name.value = val

// Cast a LargestIntegralType to pointer_type via a ValuePointer.
#define cast_largest_integral_type_to_pointer( \
    pointer_type, largest_integral_type) \
    ((pointer_type)((ValuePointer*)&(largest_integral_type))->pointer)

// Used to cast LargetIntegralType to void* and vice versa.
typedef union ValuePointer {
  LargestIntegralType value;
  void *pointer;
} ValuePointer;

//双向循环链表
struct ListNode {
    const void *value;
    int refcount;
    struct ListNode *next;
    struct ListNode *prev;
} ;

// 符号的值及其声明位置。
typedef struct SymbolValue {
  SourceLocation location;
  LargestIntegralType value;
} SymbolValue;


/* Contains a list of values for a symbol.
 * NOTE: Each structure referenced by symbol_values_list_head must have a
 * SourceLocation as its' first member.
 */
struct SymbolMapValue {
    const char *symbol_name;
    //ListNode
    ListNode symbol_values_list_head;
};

// Keeps a map of the values that functions will have to return to provide
// mocked interfaces.保留函数必须返回以提供模拟接口的值的映射。
static ListNode global_function_result_map_head;
// 声明了返回的最后一个模拟值的位置。
static SourceLocation global_last_mock_value_location;

//保留函数期望作为参数的值到其模拟接口的映射。
static ListNode global_function_parameter_map_head;
// 已声明的上次检查的参数值的位置。
static SourceLocation global_last_parameter_location;
// 所有当前分配的块的列表。
static ListNode global_allocated_blocks;

///声明本文件中定义的函数
// Used by list_free() to deallocate values referenced by list nodes.
typedef void (*CleanupListValue)(const void *value, void *cleanup_value_data);
// Determines whether two values are the same.
typedef int (*EqualityFunction)(const void *left, const void *right);
// 解除分配列表引用的值。
static void free_value(const void *value, void *cleanup_value_data) ;
// 检查测试设置的任何剩余值是否从未通过执行检索，如果是这种情况，则测试失败。
static int check_for_leftover_values(
        const ListNode * const map_head, const char * const error_message,
        const size_t number_of_symbol_names);
// 确定链表是否为空，返回head->next == head
static int list_empty(const ListNode * const head);

static ListNode* list_initialize(ListNode * const node);
static ListNode* list_add(ListNode * const head, ListNode *new_node);
static ListNode* list_add_value(ListNode * const head, const void *value,
                                const int count);
static ListNode* list_remove(
    ListNode * const node, const CleanupListValue cleanup_value,
    void * const cleanup_value_data);
static void list_remove_free(
    ListNode * const node, const CleanupListValue cleanup_value,
    void * const cleanup_value_data);
static int list_empty(const ListNode * const head);
static int list_find(
    ListNode * const head, const void *value,
    const EqualityFunction equal_func, ListNode **output);
static int list_first(ListNode * const head, ListNode **output);
static ListNode* list_free(
    ListNode * const head, const CleanupListValue cleanup_value,
    void * const cleanup_value_data);

static void add_symbol_value(
    ListNode * const symbol_map_head, const char * const symbol_names[],
    const size_t number_of_symbol_names, const void* value, const int count);
static int get_symbol_value(
    ListNode * const symbol_map_head, const char * const symbol_names[],
    const size_t number_of_symbol_names, void **output);
static void free_value(const void *value, void *cleanup_value_data);
static void free_symbol_map_value(
    const void *value, void *cleanup_value_data);
static void remove_always_return_values(ListNode * const map_head,
                                        const size_t number_of_symbol_names);
static int check_for_leftover_values(
    const ListNode * const map_head, const char * const error_message,
    const size_t number_of_symbol_names);
// This must be called at the beginning of a test to initialize some data
// structures.
static void initialize_testing(const char *test_name);
// This must be called at the end of a test to free() allocated structures.
static void teardown_testing(const char *test_name);
// Determine whether a source location is currently set.
static int source_location_is_set(const SourceLocation * const location);

// State of each test.
typedef struct TestState {
    const ListNode *check_point; // Check point of the test if there's a
    // setup function.
    void *state;                 // State associated with the test.
} TestState;
// Debug information for malloc().
struct MallocBlockInfo {
    void* block;              // Address of the block returned by malloc().
    size_t allocated_size;    // Total size of the allocated block.
    size_t size;              // Request block size.
    SourceLocation location;  // Where the block was allocated.
    ListNode node;            // Node within list of all allocated blocks.
} ;



// 用于检查整数类型范围的结构。
struct CheckIntegerRange {
  CheckParameterEvent event;
  LargestIntegralType minimum;
  LargestIntegralType maximum;
} ;

// Structure used to check whether an integer value is in a set.
struct CheckIntegerSet {
  CheckParameterEvent event;
  const LargestIntegralType *set;
  size_t size_of_set;
} ;

/* Used to check whether a parameter matches the area of memory referenced by
 * this structure.  */
typedef struct CheckMemoryData {
  CheckParameterEvent event;
  const void *memory;
  size_t size;
} CheckMemoryData;

// Determine whether a source location is currently set.
static int source_location_is_set(const SourceLocation * const location) {
  assert_true(location);
  return location->file && location->line; //两个都不为0说明已经被设置
}

static int list_empty(const ListNode * const head) {
    assert_true(head);
    return head->next == head;
}

// Initialize a list node.
static ListNode* list_initialize(ListNode * const node) {
  node->value = NULL;
  node->refcount = 1;
  node->next = node;
  node->prev = node;

  return node;
}
static ListNode* list_add(ListNode * const head, ListNode *new_node);
/* Adds a value at the tail of a given list.
 * The node referencing the value is allocated from the heap.
 * 在给定列表的尾部添加一个值。引用该值的节点是从堆中分配的。*/
static ListNode* list_add_value(ListNode * const head, const void *value,
                                const int refcount) {
  ListNode * const new_node = (ListNode*)malloc(sizeof(ListNode));
  assert_true(head);
  assert_true(value);
  new_node->value = value;
  new_node->refcount = refcount;
  return list_add(head, new_node);
}

// Add new_node to the end of the list.
static ListNode* list_add(ListNode * const head, ListNode *new_node) {
  assert_true(head);
  assert_true(new_node);
  new_node->next = head;
  new_node->prev = head->prev;
  head->prev->next = new_node;
  head->prev = new_node;
  return new_node;
}

// Remove a node from a list.
static ListNode* list_remove(
        ListNode * const node, const CleanupListValue cleanup_value,
        void * const cleanup_value_data) {
    assert_true(node);
    node->prev->next = node->next;
    node->next->prev = node->prev;
    if (cleanup_value) {
        cleanup_value(node->value, cleanup_value_data);
    }
    return node;
}

/* Remove a list node from a list and free the node. */
static void list_remove_free(
        ListNode * const node, const CleanupListValue cleanup_value,
        void * const cleanup_value_data) {
    assert_true(node);
    free(list_remove(node, cleanup_value, cleanup_value_data));
}
//释放一个list
static ListNode* list_free(
    ListNode * const head, const CleanupListValue cleanup_value,
    void * const cleanup_value_data) {
  assert_true(head);
  while (!list_empty(head))
    free(list_remove(head->next,cleanup_value,cleanup_value_data));
  return head;
}


// 解除分配列表引用的值。
static void free_value(const void *value, void *cleanup_value_data) ;



// Releases memory associated to a symbol_map_value.
static void free_symbol_map_value(const void *value,
                                  void *cleanup_value_data) {
  ///先使用一个临时知map_value保存要删除的值，然后从列表中将该想删除，最后释放内存
  SymbolMapValue * const map_value = (SymbolMapValue*)value;
  const unsigned int children =static_cast<int>(reinterpret_cast<intptr_t>(cleanup_value_data));
  ///const unsigned int children = (unsigned int)cleanup_value_data;
  assert_true(value);
  list_free(&map_value->symbol_values_list_head,
            children ? free_symbol_map_value : free_value,
            static_cast<void*>((int*)cleanup_value_data - 1));///FIXME :类型有问题
  free(map_value);
}


// Deallocate a value referenced by a list.解除分配列表引用的值。
static void free_value(const void *value, void *cleanup_value_data) {
    assert_true(value);
    free((void*)value);
}

/* 使用equal_func在列表中查找值，将每个节点与该值进行比较
 * Find a value in the list using the equal_func to compare each node with the
* value.*/
static int list_find(ListNode * const head, const void *value,
                     const EqualityFunction equal_func, ListNode **output) {
  ListNode *current;
  assert_true(head);
  for (current = head->next; current != head; current = current->next) { //循环遍历
    if (equal_func(current->value, value)) {
      *output = current;
      return 1;
    }
  }
  return 0;
}

// 返回列表的第一个节点
static int list_first(ListNode * const head, ListNode **output) {
  ListNode *target_node;
  assert_true(head);
  if (list_empty(head)) {
    return 0;
  }
  target_node = head->next;
  *output = target_node;
  return 1;
}

/* Determine whether a symbol name referenced by a symbol_map_value
 * matches the specified function name.
 * 确定symbol_map_value引用的符号名称(symbol_name)是否与指定的函数名称匹配。*/
static int symbol_names_match(const void *map_value, const void *symbol) {
  return !strcmp(((SymbolMapValue*)map_value)->symbol_name,
                 (const char*)symbol);
}

/* Adds a value to the queue of values associated with the given
 * hierarchy of symbols.  It's assumed value is allocated from the heap.
 */
static void add_symbol_value(ListNode * const symbol_map_head,
                             const char * const symbol_names[],
                             const size_t number_of_symbol_names,
                             const void* value, const int refcount) {
  const char* symbol_name;
  ListNode *target_node;
  SymbolMapValue *target_map_value;
  assert_true(symbol_map_head);
  assert_true(symbol_names);
  assert_true(number_of_symbol_names);
  symbol_name = symbol_names[0];

  if (!list_find(symbol_map_head, symbol_name, symbol_names_match,
                 &target_node)) {
    SymbolMapValue * const new_symbol_map_value =static_cast<SymbolMapValue *>(
        malloc(sizeof(*new_symbol_map_value)));
    new_symbol_map_value->symbol_name = symbol_name;
    list_initialize(&new_symbol_map_value->symbol_values_list_head);
    target_node = list_add_value(symbol_map_head, new_symbol_map_value,
                                 1);
  }

  target_map_value = (SymbolMapValue*)target_node->value;
  if (number_of_symbol_names == 1) {
    list_add_value(&target_map_value->symbol_values_list_head,
                   value, refcount);
  } else {
    add_symbol_value(&target_map_value->symbol_values_list_head,
                     &symbol_names[1], number_of_symbol_names - 1, value,
                     refcount);
  }
}


/* Traverse down a tree of symbol values and remove the first symbol value
 * in each branch that has a refcount < -1 (i.e should always be returned
 * and has been returned at least once).
 */
//用于从链表中删除总是返回值的项。函数有两个参数：指向链表头的指针和符号名称的数量。
static void remove_always_return_values(ListNode * const map_head,
                                        const size_t number_of_symbol_names) {
    ListNode *current;
    assert_true(map_head);
    assert_true(number_of_symbol_names);
    current = map_head->next;
    while (current != map_head) { //遍历，Listnode是一个双向循环链表
        SymbolMapValue * const value = (SymbolMapValue*)current->value;
        ListNode * const next = current->next;
        ListNode *child_list;
        assert_true(value);
        child_list = &value->symbol_values_list_head;

        if (!list_empty(child_list)) {
            if (number_of_symbol_names == 1) {
                ListNode * const child_node = child_list->next;
                // If this item has been returned more than once, free it.
                if (child_node->refcount < -1) {
                    list_remove_free(child_node, free_value, NULL);
                }
            } else {
                remove_always_return_values(child_list,
                                            number_of_symbol_names - 1);
            }
        }

        if (list_empty(child_list)) {
            list_remove_free(current, free_value, NULL);
        }
        current = next;
    }
}
// exception_handler() 捕获的信号。
static const int exception_signals[] = {
        SIGFPE,  //浮点异常，例如除以零或溢出。
        SIGILL,  //非法指令，例如执行了未定义或特权的指令。
        SIGSEGV, //段错误，例如访问了无效或受保护的内存地址。
        SIGBUS,  //总线错误，例如对齐错误或物理内存故障。
        SIGSYS,  //无效的系统调用，例如传递了错误的参数或调用了未实现的功能。
};
// Default signal functions that should be restored after a test is complete.测试完成后应恢复的默认信号功能。
typedef void (*SignalFunction)(int signal);
static SignalFunction default_signal_functions[
        ARRAY_LENGTH(exception_signals)];

// Keeps track of the calling context returned by setenv() so that the fail()
// method can jump out of a test.
static jmp_buf global_run_test_env;
static int global_running_test = 0;
// Keeps track of the calling context returned by setenv() so that
// mock_assert() can optionally jump back to expect_assert_failure().
jmp_buf global_expect_assert_env;
int global_expecting_assert = 0;
// 退出当前正在执行的测试，会尝试恢复开始测试之前保存的环境变量
// 如果之前没有保存成功，则调用exit(-1)函数退出测试
static void exit_test(const int quit_application) {
    if (global_running_test) { //如果是被测程序出现错误，恢复测试测序开始之前的状态，不退出测试，否则
        longjmp(global_run_test_env, 1);//用于恢复之前由setjmp保存的栈环境和执行位置
        //并传入一个全局变量global_run_test_env作为保存环境的对象，以及一个整数1作为从setjmp返回的值。
        // 这意味着程序会跳转到之前调用setjmp并保存global_run_test_env对象的地方，并从setjmp返回1。
    } else if (quit_application) {
        exit(-1);
    }
}

// Initialize a SourceLocation structure.初始化源位置结构。
static void initialize_source_location(SourceLocation * const location) {
    assert_true(location);
    location->file = NULL;
    location->line = 0;
}
// Set a source location.
static void set_source_location(
    SourceLocation * const location, const char * const file,
    const int line) {
  assert_true(location);
  location->file = file;
  location->line = line;
}

// 创建函数结果和预期参数列表。
// Create function results and expected parameter lists.
void initialize_testing(const char *test_name) {
    list_initialize(&global_function_result_map_head);
    initialize_source_location(&global_last_mock_value_location);
    list_initialize(&global_function_parameter_map_head);
    initialize_source_location(&global_last_parameter_location);
}

// Get the list of allocated blocks.获得已分配的块的列表,在每次改变测试函数之前释放，并检测是否发生内存泄漏
static ListNode* get_allocated_blocks_list() {
    // If it initialized, initialize the list of allocated blocks.
    if (!global_allocated_blocks.value) {
        list_initialize(&global_allocated_blocks);
        global_allocated_blocks.value = (void*)1;
    }
    return &global_allocated_blocks;
}
// Crudely checkpoint the current heap state.
static const ListNode* check_point_allocated_blocks() {
    return get_allocated_blocks_list()->prev;
}

//TODO：实现vsnprintf
// Standard output and error print methods.
void vprint_message(const char* const format, va_list args) {
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    std::cout<<buffer<<std::flush;
    //fputs(buffer,stdout);
}
void vprint_error(const char* const format, va_list args) {
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    std::cerr<<buffer;
    //fputs(buffer,stderr);
}
//TODO : 使用initial_list<>实现可变参数的读取
void print_message(const char* const format, ...) {
    va_list args;
    va_start(args, format);
    vprint_message(format,args);
    va_end(args);
}

void print_error(const char* const format, ...) {
    va_list args; //定义一个可变参数列表
    va_start(args, format);//初始化参数列表，传入最后一个固定参数
    vprint_error(format,args);
    va_end(args); //释放内存
}



/* Determine whether value is contained within check_integer_set.
 * If invert is 0 and the value is in the set 1 is returned, otherwise 0 is
 * returned and an error is displayed.  If invert is 1 and the value is not
 * in the set 1 is returned, otherwise 0 is returned and an error is
 * displayed. */
static int value_in_set_display_error(
    const LargestIntegralType value,
    const CheckIntegerSet * const check_integer_set, const int invert) {
  int succeeded = invert;
  assert_true(check_integer_set);
  {
    const LargestIntegralType * const set = check_integer_set->set;
    const size_t size_of_set = check_integer_set->size_of_set;
    size_t i;
    for (i = 0; i < size_of_set; i++) {
      if (set[i] == value) {
        // If invert = 0 and item is found, succeeded = 1.
        // If invert = 1 and item is found, succeeded = 0.
        succeeded = !succeeded;
        break;
      }
    }
    if (succeeded) {
      return 1;
    }
    print_error("%d is %sin the set (", value, invert ? "" : "not ");
    for (i = 0; i < size_of_set; i++) {
      print_error("%d, ", set[i]);
    }
    print_error(")\n");
  }
  return 0;
}


/* Determine whether a value is within the specified range.  If the value is
 * within the specified range 1 is returned.  If the value isn't within the
 * specified range an error is displayed and 0 is returned. */
static int integer_in_range_display_error(
    const LargestIntegralType value, const LargestIntegralType range_min,
    const LargestIntegralType range_max) {
  if (value >= range_min && value <= range_max) {
    return 1;
  }
  print_error("%d is not within the range %d-%d\n", value, range_min,
              range_max);
  return 0;
}


/* Determine whether a value is within the specified range.  If the value
 * is not within the range 1 is returned.  If the value is within the
 * specified range an error is displayed and zero is returned. */
static int integer_not_in_range_display_error(
    const LargestIntegralType value, const LargestIntegralType range_min,
    const LargestIntegralType range_max) {
  if (value < range_min || value > range_max) {
    return 1;
  }
  print_error("%d is within the range %d-%d\n", value, range_min,
              range_max);
  return 0;
}


/* Determine whether the specified strings are equal.  If the strings are equal
 * 1 is returned.  If they're not equal an error is displayed and 0 is
 * returned. */
static int string_equal_display_error(
    const char * const left, const char * const right) {
  if (strcmp(left, right) == 0) {  ///TODO:使用compare实现字符串的比较
    return 1;
  }
  print_error("\"%s\" != \"%s\"\n", left, right);
  return 0;
}


/* Determine whether the specified strings are equal.  If the strings are not
 * equal 1 is returned.  If they're not equal an error is displayed and 0 is
 * returned */
static int string_not_equal_display_error(
    const char * const left, const char * const right) {
  if (strcmp(left, right) != 0) {
    return 1;
  }
  print_error("\"%s\" == \"%s\"\n", left, right);
  return 0;
}


/* Returns 1 if the specified values are equal.  If the values are not equal
 * an error is displayed and 0 is returned. */
static int values_equal_display_error(const LargestIntegralType left,
                                      const LargestIntegralType right) {
  const int equal = left == right;
  if (!equal) {
    print_error(LargestIntegralTypePrintfFormat " != "
                LargestIntegralTypePrintfFormat "\n", left, right);
  }
  return equal;
}

/* Determine whether the specified areas of memory are equal.  If they're equal
 * 1 is returned otherwise an error is displayed and 0 is returned. */
static int memory_equal_display_error(const char* const a, const char* const b,
                                      const size_t size) {
  int differences = 0;
  size_t i;
  for (i = 0; i < size; i++) {
    const char l = a[i];
    const char r = b[i];
    if (l != r) {
      print_error("difference at offset %d 0x%02x 0x%02x\n", i, l, r);
      differences ++;
    }
  }
  if (differences) {
    print_error("%d bytes of 0x%08x and 0x%08x differ\n", differences,
                a, b);
    return 0;
  }
  return 1;
}


/* Determine whether the specified areas of memory are not equal.  If they're
 * not equal 1 is returned otherwise an error is displayed and 0 is
 * returned. */
static int memory_not_equal_display_error(
    const char* const a, const char* const b, const size_t size) {
  int same = 0;
  size_t i;
  for (i = 0; i < size; i++) {
    const char l = a[i];
    const char r = b[i];
    if (l == r) {
      same ++;
    }
  }
  if (same == size) {
    print_error("%d bytes of 0x%08x and 0x%08x the same\n", same,
                a, b);
    return 0;
  }
  return 1;
}

static void exception_handler(int sig) {
    print_error("%s\n", strsignal(sig));
    exit_test(1);
}

int _run_test(
        const std::string  function_name, const UnitTestFunction Function,
        void ** const state, const UnitTestFunctionType function_type,
        const void* const heap_check_point) {
    const ListNode * const check_point = static_cast<const ListNode *>(heap_check_point ?
                                            heap_check_point : check_point_allocated_blocks());
    void *current_state = NULL;
    int rc = 1;
    int handle_exceptions = 1;
#if UNIT_TESTING_DEBUG
    handle_exceptions = 0;
#endif // UNIT_TESTING_DEBUG

    if (handle_exceptions) {
        unsigned int i;
        for (i = 0; i < ARRAY_LENGTH(exception_signals); i++) {
            default_signal_functions[i] = signal(
                    exception_signals[i], exception_handler);
        }
    }
    //输出提示信息
    if (function_type == UNIT_TEST_FUNCTION_TYPE_TEST) {
        std::cout<<function_name<<": Starting test\n";
    }
    initialize_testing(function_name.c_str());
    return rc;
}

#undef malloc

void fail_if_leftover_values(const char *const name) {
    int error_occurred = 0;//标记是否发生错误
    remove_always_return_values(&global_function_result_map_head, 1);
    if (check_for_leftover_values(            //检查
            &global_function_result_map_head,
            "%s() has remaining non-returned values.\n", 1)) {
        error_occurred = 1;
    }

    remove_always_return_values(&global_function_parameter_map_head, 2);
    if (check_for_leftover_values(
            &global_function_parameter_map_head,
            "%s parameter still has values that haven't been checked.\n", 2)) {
        error_occurred = 1;
    }
    if (error_occurred) {
        exit_test(1);
    }
}
// 检查测试设置的任何剩余值是否从未通过执行检索，如果是这种情况，则测试失败。
static int check_for_leftover_values(
        const ListNode * const map_head, const char * const error_message,
        const size_t number_of_symbol_names) {
    const ListNode *current;
    int symbols_with_leftover_values = 0;
    assert_true(map_head);
    assert_true(number_of_symbol_names);

    for (current = map_head->next; current != map_head;
            current = current->next) { //遍历List
        const SymbolMapValue * const value =
                static_cast<const SymbolMapValue*>(current->value);
        const ListNode *child_list;
        assert_true(value);
        child_list = &value->symbol_values_list_head;

        if (!list_empty(child_list)) {
            if (number_of_symbol_names == 1) {
                const ListNode *child_node;
                print_error(error_message, value->symbol_name);
                print_error("  Remaining item(s) declared at...\n");

                for (child_node = child_list->next; child_node != child_list;
                        child_node = child_node->next) {
                    const SourceLocation * const location =
                            static_cast<const SourceLocation * const>(child_node->value);
                    print_error("    " SOURCE_LOCATION_FORMAT "\n",
                                location->file, location->line);
                }
            } else {
                print_error("%s.", value->symbol_name);
                check_for_leftover_values(child_list, error_message,
                                            number_of_symbol_names - 1);
            }
            symbols_with_leftover_values ++;
        }
    }
    return symbols_with_leftover_values;
}
// Get the next return value for the specified mock function.获取指定模拟函数的下一个返回值。
LargestIntegralType _mock(const char * const function, const char* const file,
                          const int line) {
  void *result;
  const int rc = get_symbol_value(&global_function_result_map_head,
                                  &function, 1, &result);
  if (rc) {
    SymbolValue * const symbol = (SymbolValue*)result;
    const LargestIntegralType value = symbol->value;
    global_last_mock_value_location = symbol->location;
    if (rc == 1) { //rc == 1 表示该值刚从global_function_result_map_head中删除，
      free(symbol);
    }
    return value;
  } else { //没有找到
    print_error("ERROR: " SOURCE_LOCATION_FORMAT " - Could not get value "
                "to mock function %s\n", file, line, function);
    if (source_location_is_set(&global_last_mock_value_location)) {
      print_error("Previously returned mock value was declared at "
                  SOURCE_LOCATION_FORMAT "\n",
                  global_last_mock_value_location.file,
                  global_last_mock_value_location.line);
    } else {
      print_error("There were no previously returned mock values for "
                  "this test.\n");
    }
    exit_test(1);
  }
  return 0;
}
/* Display the blocks allocated after the specified check point.  This
 * function returns the number of blocks displayed. */
static int display_allocated_blocks(const ListNode * const check_point) {
    const ListNode * const head = get_allocated_blocks_list();
    const ListNode *node;
    int allocated_blocks = 0;
    assert_true(check_point);
    for (node = check_point->next; node != head; node = node->next) {
        const MallocBlockInfo * const block_info =
                (const MallocBlockInfo*)node->value;
        assert_true(block_info);

        if (!allocated_blocks) {
            print_error("Blocks allocated...\n");
        }
        print_error("  0x%08" PRIxMAX " : " SOURCE_LOCATION_FORMAT "\n",
                    cast_to_largest_integral_type(block_info->block),
                block_info->location.file,
                block_info->location.line);
        allocated_blocks ++;
    }
    return allocated_blocks;

}


// 释放在指定检查点之后分配的所有块。
// Free all blocks allocated after the specified check point.
static void free_allocated_blocks(const ListNode * const check_point) {
    const ListNode * const head = get_allocated_blocks_list();
    const ListNode *node;
    assert_true(check_point);

    node = check_point->next;
    assert_true(node);

    while (node != head) {
        MallocBlockInfo * const block_info = (MallocBlockInfo*)node->value;
        node = node->next;
        free((char*)block_info + sizeof(*block_info) + MALLOC_GUARD_SIZE);
    }
}
// 如果在一个测试结束后，仍然有未分配的块，则为内存泄漏
// Fail if any any blocks are allocated after the specified check point.
static void fail_if_blocks_allocated(const ListNode * const check_point,
                              const char * const test_name) {
    const int allocated_blocks = display_allocated_blocks(check_point);
    if (allocated_blocks) {
        free_allocated_blocks(check_point);
        print_error("ERROR: %s leaked %d block(s)\n", test_name,
                    allocated_blocks);
        exit_test(1);
    }
}
//把当前测试函数对应的参数和结果从对应的队列中删除，并声明返回的最后一个模拟值和检查的参数的值
void teardown_testing(const char *const name) {
  list_free(&global_function_result_map_head, free_symbol_map_value,
            (void*)0);
  initialize_source_location(&global_last_mock_value_location);
  list_free(&global_function_parameter_map_head, free_symbol_map_value,
            (void*)1);
  initialize_source_location(&global_last_parameter_location);
}

static int get_symbol_value(ListNode *const head, const char *const *symbol_names, const size_t number_of_symbol_names,
                           void **output) {
    const char* symbol_name;
    ListNode *target_node;

    assert_true(head);
    assert_true(symbol_names);
    assert_true(number_of_symbol_names);

    symbol_name = symbol_names[0];
    if (list_find(head, symbol_name, symbol_names_match, &target_node)) {
        SymbolMapValue *map_value;
        ListNode *child_list;
        int return_value = 0;

        map_value = (SymbolMapValue*)target_node->value;
        child_list = &map_value->symbol_values_list_head;
        if (number_of_symbol_names == 1) {
            ListNode *value_node = NULL;
            return_value = list_first(child_list, &value_node);
            assert_true(return_value);
            *output = (void*) value_node->value;
            return_value = value_node->refcount;
            if (--value_node->refcount == 0) {
                list_remove_free(value_node, NULL, NULL);
            }
        } else {
            return_value = get_symbol_value(
                    child_list, &symbol_names[1], number_of_symbol_names - 1,
                    output);
        }
        if (list_empty(child_list)) {
            list_remove_free(target_node, free_symbol_map_value, (void*)0);
        }
        return return_value;
    } else {
        print_error("No entries for symbol %s.\n", symbol_name);
    }
    return 0;

}

int _run_test(
        const char * const function_name, const UnitTestFunction Function,
        void ** const state, const UnitTestFunctionType function_type,
        const void* const heap_check_point) {
    const ListNode * const check_point = heap_check_point ?
                  static_cast<const ListNode *>(heap_check_point): check_point_allocated_blocks();
    void *current_state = NULL;
    int rc = 1;
    int handle_exceptions = 1;
#if UNIT_TESTING_DEBUG
    handle_exceptions = 0;
#endif // UNIT_TESTING_DEBUG
    if (handle_exceptions) {
        unsigned int i;
        for (i = 0; i < sizeof(exception_signals); i++) {
            default_signal_functions[i] = signal(
                    exception_signals[i], exception_handler);
        }
    }
    if (function_type == UNIT_TEST_FUNCTION_TYPE_TEST) {
        print_message("[ RUN      ] %s\n", function_name);
    }
    initialize_testing(function_name);
    global_running_test = 1;
    if (setjmp(global_run_test_env) == 0) {
        struct timeval time_start, time_end;
        gettimeofday(&time_start, NULL);

        //执行测试
        Function(state ? state : &current_state);

        // Collect time data
        gettimeofday(&time_end, NULL);

        //检查是否已经将全部参数和返回值测试完
        fail_if_leftover_values(function_name);

        /* 如果这是一个设置函数，则忽略任何分配的块，仅确保它们在拆卸时被释放
         * 如果不是，检测是否发生内存泄漏
         * If this is a setup function then ignore any allocated blocks
         * only ensure they're deallocated on tear down. */
        if (function_type != UNIT_TEST_FUNCTION_TYPE_SETUP) {
            fail_if_blocks_allocated(check_point, function_name);
        }

        global_running_test = 0;

        if (function_type == UNIT_TEST_FUNCTION_TYPE_TEST) {
            print_message("[       OK ] %s\n", function_name);
        }
        rc = 0;
    } else {
        global_running_test = 0;
        ///TODO:新的一种测试类型
       /* if(UNIT_TEST_FUNCTION_TYPE_TEST_EXPECT_FAILURE == function_type) {
            rc = 0;
            print_message("[EXPCT FAIL] %s\n", function_name);
        } else {)*/

        print_message("[  FAILED  ] %s\n", function_name);
    }
    teardown_testing(function_name);

    return rc;
}
int _run_tests(const UnitTest * const tests, const size_t number_of_tests) {
    // 是否执行下一个测试
    int run_next_test = 1;
    // 上个测试是否失败
    int previous_test_failed = 0;
    // Check point of the heap state.
    const ListNode * const check_point = check_point_allocated_blocks();
    // Current test being executed.
    size_t current_test = 0;
    // Number of tests executed.
    size_t tests_executed = 0;
    // Number of failed tests.
    size_t total_failed = 0;
    // Number of setup functions.设置功能的数量
    size_t setups = 0;
    // Number of teardown functions.拆解功能的数量
    size_t teardowns = 0;
    /* A stack of test states.  A state is pushed on the stack
     * when a test setup occurs and popped on tear down. */
    /// TODO: 实现使用new操作符的auto test_states = new TestState[number_of_tests];
    size_t number_of_test_states = 0;
    TestState* test_states = static_cast<TestState *>(
        malloc(number_of_tests * sizeof(*test_states)));
    // Names of the tests that failed.
    std::vector<std::string> failed_names(number_of_tests);
    void **current_state = nullptr;
    ///const char** failed_names = malloc(number_of_tests *
    ///                                   sizeof(*failed_names));

    // Make sure LargestIntegralType is at least the size of a pointer.
    assert_true(sizeof(LargestIntegralType) >= sizeof(void*));

    while (current_test < number_of_tests) { //通过循环完成所有测试
        const ListNode *test_check_point = nullptr;
        TestState *current_TestState;
        const UnitTest * const test = &tests[current_test++];
        if (!test->function) {
            continue;
        }

        switch (test->function_type) {
            case UNIT_TEST_FUNCTION_TYPE_TEST:
                run_next_test = 1;
                break;
            case UNIT_TEST_FUNCTION_TYPE_SETUP: {
                //setup不执行，而是设置
                current_TestState = &test_states[number_of_test_states++];
                // Checkpoint the heap before the setup.
                current_TestState->check_point = check_point_allocated_blocks();
                test_check_point = current_TestState->check_point;
                current_state = &current_TestState->state;
                *current_state = NULL;
                run_next_test = 1;
                setups++;
                break;
            }
            case UNIT_TEST_FUNCTION_TYPE_TEARDOWN:
                // Check the heap based on the last setup checkpoint.
                assert_true(number_of_test_states);
                current_TestState = &test_states[--number_of_test_states];
                test_check_point = current_TestState->check_point;
                current_state = &current_TestState->state;
                teardowns++;
                break;
            default:
                print_error("Invalid unit test function type %d\n",test->function_type);
                exit_test(1);
                break;
        }
        if (run_next_test) {
            int failed = _run_test(test->name, test->function, current_state,\
                                    test->function_type, test_check_point);
            if (failed) {
                auto testname = test->name;
                failed_names[total_failed] = testname;
            }

            switch (test->function_type) {
                case UNIT_TEST_FUNCTION_TYPE_TEST:
                    previous_test_failed = failed;
                    total_failed += failed;
                    tests_executed ++;
                    break;

                case UNIT_TEST_FUNCTION_TYPE_SETUP:
                    if (failed) {
                        total_failed ++;
                        tests_executed ++;
                        // Skip forward until the next test or setup function.
                        run_next_test = 0;
                    }
                    previous_test_failed = 0;
                    break;

                case UNIT_TEST_FUNCTION_TYPE_TEARDOWN:
                    // If this test failed.
                    if (failed && !previous_test_failed) {
                        total_failed ++;
                    }
                    break;
                default:
                    assert_false("BUG: shouldn't be here!");
                    break;
            }
        }
    }
    if (total_failed) {
      size_t i;
      print_error("%d out of %d tests failed!\n", total_failed,
                  tests_executed);
      for (i = 0; i < total_failed; i++) {
        print_error("    %s\n", (failed_names[i].c_str()));
      }
    } else {
      print_message("All %d tests passed\n", tests_executed);
    }
    return (int)total_failed;
}

// CheckParameterValue callback to check whether a parameter equals a string.
static int check_string(const LargestIntegralType value,
                        const LargestIntegralType check_value_data) {
  return string_equal_display_error(
      cast_largest_integral_type_to_pointer(char*, value),
      cast_largest_integral_type_to_pointer(char*, check_value_data));
}
/* 添加自定义参数检查功能。如果事件参数为 NULL，则事件结构由此函数在内部分配。如果提供了事件参
 * 数，则必须在堆上分配该参数，并且调用方不需要将其解除分配。
 * Add a custom parameter checking function.  If the event parameter is NULL
 * the event structure is allocated internally by this function.  If event
 * parameter is provided it must be allocated on the heap and doesn't need to
 * be deallocated by the caller.
 */
void _expect_check(
    const char* const function, const char* const parameter,
    const char* const file, const int line,
    const CheckParameterValue check_function,
    const LargestIntegralType check_data, CheckParameterEvent * const event,
    const int count) {
  CheckParameterEvent * const check =
      event ? event : static_cast<CheckParameterEvent *>(malloc(sizeof(*check)));
  const char* symbols[] = {function, parameter};
  check->parameter_name = parameter;
  check->check_value = check_function;
  check->check_value_data = check_data;
  set_source_location(&check->location, file, line);
  add_symbol_value(&global_function_parameter_map_head, symbols, 2, check,
                   count);

}
///TODO: 使用函数对象或标准库替代
/* CheckParameterValue callback to check whether a value is equal to an
 * expected value. */
static int check_value(const LargestIntegralType value,
                       const LargestIntegralType check_value_data) {
  return values_equal_display_error(value, check_value_data);
}

// 添加事件以检查参数是否等于预期值。
void _expect_value(
    const char* const function, const char* const parameter,
    const char* const file, const int line,
    const LargestIntegralType value, const int count) {
  _expect_check(function, parameter, file, line, check_value, value, NULL,
                count);
}
//添加事件以检查参数是否等于字符串。
void _expect_string(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const char* string,
    const int count) {
  declare_initialize_value_pointer_pointer(string_pointer, (char*)string);
  _expect_check(function, parameter, file, line, check_string,
                string_pointer.value, NULL, count);
}
void _expect_not_string(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const char* string,
    const int count);

void _expect_memory(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const void* const memory,
    const size_t size, const int count);
void _expect_not_memory(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const void* const memory,
    const size_t size, const int count);

// CheckParameterValue callback that always returns 1.
static int check_any(const LargestIntegralType value,
                     const LargestIntegralType check_value_data) {
  return 1;
}
// 添加事件以允许参数的任何值。
void _expect_any(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const int count) {
  _expect_check(function, parameter, file, line, check_any, 0, NULL,
                count);
}

/* 获取与给定符号层次结构关联的下一个值。该值作为输出参数返回，如果找到值，函数将返回节点的
 * 旧 refcount 值，否则为 0。这意味着返回值 1 表示该节点刚刚从列表中删除。
 * Gets the next value associated with the given hierarchy of symbols.
 * The value is returned as an output parameter with the function returning the
 * node's old refcount value if a value is found, 0 otherwise.
 * This means that a return value of 1 indicates the node was just removed from
 * the list.
 */


void _check_expected(
    const char * const function_name, const char * const parameter_name,
    const char* file, const int line, const LargestIntegralType value) {
  void *result;
  const char* symbols[] = {function_name, parameter_name};
  const int rc = get_symbol_value(&global_function_parameter_map_head,
                                  symbols, 2, &result);
  if (rc) {
    CheckParameterEvent * const check = (CheckParameterEvent*)result;
    int check_succeeded;
    global_last_parameter_location = check->location;
    check_succeeded = check->check_value(value, check->check_value_data);
    if (rc == 1) {
      free(check);
    }
    if (!check_succeeded) {
      print_error("ERROR: Check of parameter %s, function %s failed\n"
                  "Expected parameter declared at "
                  SOURCE_LOCATION_FORMAT "\n",
                  parameter_name, function_name,
                  global_last_parameter_location.file,
                  global_last_parameter_location.line);
      _fail(file, line);
    }
  } else {
    print_error("ERROR: " SOURCE_LOCATION_FORMAT " - Could not get value "
                "to check parameter %s of function %s\n", file, line,
                parameter_name, function_name);
    if (source_location_is_set(&global_last_parameter_location)) {
      print_error("Previously declared parameter value was declared at "
                  SOURCE_LOCATION_FORMAT "\n",
                  global_last_parameter_location.file,
                  global_last_parameter_location.line);
    } else {
      print_error("There were no previously declared parameter values "
                  "for this test.\n");
    }
    exit_test(1);
  }
}
//替换assert
void mock_assert(const int result, const char* const expression,
                 const char * const file, const int line) {
  if (!result) {
    if (global_expecting_assert) {
      longjmp(global_expect_assert_env, (LargestIntegralType)expression);
    } else {
      print_error("ASSERT: %s\n", expression);
      _fail(file, line);
    }
  }
}

//函数对应的返回值由value指出，存储到global_function_result_map_head中去
void _will_return(const char * const function_name, const char * const file,
                  const int line, const LargestIntegralType value,
                  const int count) {
  SymbolValue * const return_value =
      static_cast<SymbolValue *const>(malloc(sizeof(*return_value)));
  assert_true(count > 0 || count == -1);
  return_value->value = value;
  set_source_location(&return_value->location, file, line);
  add_symbol_value(&global_function_result_map_head, &function_name, 1,
                   return_value, count);
}

void _assert_true(const LargestIntegralType result,
                    const char * const expression,
                    const char * const file, const int line) {
    if (!result) {
        print_error("expected '%s' to be true\n", expression);
        _fail(file, line);
    }
}
void _assert_false(const LargestIntegralType result,
                    const char * const expression,
                    const char * const file, const int line) {
    if (!result) {
        print_error("expected '%s' to be false\n", expression);
        _fail(file, line);
    }
}
void _assert_int_equal(
    const LargestIntegralType a, const LargestIntegralType b,
    const char * const file, const int line) {
    if(a!=b) {
      print_error(LargestIntegralTypePrintfFormat " != "
                  LargestIntegralTypePrintfFormat "\n", a, b);
      _fail(file, line);
    }
}
void _assert_int_not_equal(
    const LargestIntegralType a, const LargestIntegralType b,
    const char * const file, const int line) {
  if(a==b) {
    print_error(LargestIntegralTypePrintfFormat " == "
                LargestIntegralTypePrintfFormat "\n", a, b);
    _fail(file, line);
  }
}
void _assert_string_equal(const char * const a, const char * const b,
                          const char * const file, const int line) {
  if(strcmp(a,b)!=0) {
    print_error("\"%s\" != \"%s\"\n", a, b);
    _fail(file,line);
  }

}
void _assert_string_not_equal(const char * const a, const char * const b,
                              const char *file, const int line) {
  if(strcmp(a,b)==0) {
    print_error("\"%s\" == \"%s\"\n", a, b);
    _fail(file,line);
  }
  print_error("\"%s\" == \"%s\"\n", a, b);
}
// Use the real malloc in this function.
#undef malloc   //TODO: 新建一个_test_new测试函数
//在malloc之前在分配的地址前后设置一段保护块，并将分配的内存加入已分配内存列表中
void* _test_malloc(const size_t size, const char* file, const int line)  {
  char* ptr;
  MallocBlockInfo *block_info;
  ListNode * const block_list = get_allocated_blocks_list();
  const size_t allocate_size = size + (MALLOC_GUARD_SIZE * 2) +
                                sizeof(*block_info) + MALLOC_ALIGNMENT;
  char* const block = (char*)malloc(allocate_size);
  assert_true(block);

  // Calculate the returned address.
  //这段代码的作用是将block指针加上一定的偏移量，并将结果向下舍入到MALLOC_ALIGNMENT的倍数。
  ptr = (char*)(((size_t)block + MALLOC_GUARD_SIZE + sizeof(*block_info) +
                  MALLOC_ALIGNMENT) & ~(MALLOC_ALIGNMENT - 1));

  // Initialize the guard blocks.
  memset(ptr - MALLOC_GUARD_SIZE, MALLOC_GUARD_PATTERN, MALLOC_GUARD_SIZE);
  //memset将一段内存设置为指定的值
  memset(ptr + size, MALLOC_GUARD_PATTERN, MALLOC_GUARD_SIZE);
  memset(ptr, MALLOC_ALLOC_PATTERN, size);

  block_info = (MallocBlockInfo*)(ptr - (MALLOC_GUARD_SIZE +
                                          sizeof(*block_info)));
  set_source_location(&block_info->location, file, line);
  block_info->allocated_size = allocate_size;
  block_info->size = size;
  block_info->block = block;
  block_info->node.value = block_info;
  list_add(block_list, &block_info->node);
  return ptr;
}
#define malloc test_malloc

#undef new
void * _test_new(const size_t size, const char* file, const int line) {
  char* ptr;
  MallocBlockInfo *block_info;
  ListNode * const block_list = get_allocated_blocks_list();
  const size_t allocate_size = size + (MALLOC_GUARD_SIZE * 2) +
                               sizeof(*block_info) + MALLOC_ALIGNMENT;
  char*  block{nullptr};
  try {
    block = (char* const)::operator new(allocate_size);
    block = const_cast<char * const>(block);
    if(block== nullptr) { //new分配内存失败，返回空指针
      throw std::bad_alloc();
    }
  }
  catch (const std::bad_alloc &e) {
    std::cerr<<"使用operator new分配地址失败"<<e.what();
  }
  catch (std::exception & exc) {
    std::cerr<<exc.what();
  }
//  assert_true(block);

  // Calculate the returned address.
  //这段代码的作用是将block指针加上一定的偏移量，并将结果向下舍入到MALLOC_ALIGNMENT的倍数。
  ptr = (char*)(((size_t)block + MALLOC_GUARD_SIZE + sizeof(*block_info) +
                  MALLOC_ALIGNMENT) & ~(MALLOC_ALIGNMENT - 1));

  // Initialize the guard blocks.
  memset(ptr - MALLOC_GUARD_SIZE, MALLOC_GUARD_PATTERN, MALLOC_GUARD_SIZE);
  //memset将一段内存设置为指定的值
  memset(ptr + size, MALLOC_GUARD_PATTERN, MALLOC_GUARD_SIZE);
  memset(ptr, MALLOC_ALLOC_PATTERN, size);

  block_info = (MallocBlockInfo*)(ptr - (MALLOC_GUARD_SIZE +
                                          sizeof(*block_info)));
  set_source_location(&block_info->location, file, line);
  block_info->allocated_size = allocate_size;
  block_info->size = size;
  block_info->block = block;
  block_info->node.value = block_info;
  list_add(block_list, &block_info->node);
  return ptr;
}
#define new test_new

#undef delete
void _test_delete(void* const ptr, const char* file, const int line) {
  unsigned int i;
  char *block = (char*)ptr;
  MallocBlockInfo *block_info;
  _assert_true(reinterpret_cast<LargestIntegralType>(&ptr), "ptr", file, line);
  block_info = (MallocBlockInfo*)(block - (MALLOC_GUARD_SIZE +
                                            sizeof(*block_info)));
  // Check the guard blocks.
  {
    char *guards[2] = {block - MALLOC_GUARD_SIZE,
                       block + block_info->size};
    for (i = 0; i < ARRAY_LENGTH(guards); i++) {
      unsigned int j;
      char * const guard = guards[i];
      for (j = 0; j < MALLOC_GUARD_SIZE; j++) {
        const char diff = guard[j] - MALLOC_GUARD_PATTERN;
        if (diff) {
          print_error(
              "Guard block of 0x%08x size=%d allocated by "
              SOURCE_LOCATION_FORMAT " at 0x%08x is corrupt\n",
              (size_t)ptr, block_info->size,
              block_info->location.file, block_info->location.line,
              (size_t)&guard[j]);
          _fail(file, line);
        }
      }
    }
  }
  list_remove(&block_info->node, NULL, NULL);

  block = static_cast<char *>(block_info->block);
  memset(block, MALLOC_FREE_PATTERN, block_info->allocated_size);
  delete(block);
}


// Use the real free in this function.
#undef free
void _test_free(void* const ptr, const char* file, const int line) {
    unsigned int i;
    char *block = (char*)ptr;
    MallocBlockInfo *block_info;
    _assert_true(reinterpret_cast<LargestIntegralType>(&ptr), "ptr", file, line);
    block_info = (MallocBlockInfo*)(block - (MALLOC_GUARD_SIZE +
                                                sizeof(*block_info)));
    // Check the guard blocks.
    {
        char *guards[2] = {block - MALLOC_GUARD_SIZE,
                           block + block_info->size};
        for (i = 0; i < ARRAY_LENGTH(guards); i++) {
            unsigned int j;
            char * const guard = guards[i];
            for (j = 0; j < MALLOC_GUARD_SIZE; j++) {
                const char diff = guard[j] - MALLOC_GUARD_PATTERN;
                if (diff) {
                    print_error(
                            "Guard block of 0x%08x size=%d allocated by "
                            SOURCE_LOCATION_FORMAT " at 0x%08x is corrupt\n",
                            (size_t)ptr, block_info->size,
                            block_info->location.file, block_info->location.line,
                            (size_t)&guard[j]);
                    _fail(file, line);
                }
            }
        }
    }
    list_remove(&block_info->node, NULL, NULL);

    block = static_cast<char *>(block_info->block);
    memset(block, MALLOC_FREE_PATTERN, block_info->allocated_size);
    free(block);
}
#define free test_free
//调用_test_malloc
void* _test_calloc(const size_t number_of_elements, const size_t size,
                   const char* file, const int line) {
  void* const ptr = _test_malloc(number_of_elements * size, file, line);
  if (ptr) {
    memset(ptr, 0, number_of_elements * size);
  }
  return ptr;
}

void _fail(const char * const file, const int line) {
    ///std::cerr<<"ERROR"<<file<<":"<<line<<"Failure"<<std::endl;
    print_error("ERROR: " SOURCE_LOCATION_FORMAT " Failure!\n", file, line);
    exit_test(1);
}

void * malloc_test(const size_t number_of_elements, const size_t size,\
                        const char* file, const int line) {
    return NULL;
}
