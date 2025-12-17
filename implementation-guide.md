# HPPS Assignment Implementation Guide

This document extrapolates from Assignment 3 (k-NN) to provide general guidance for approaching HPPS-style C programming assignments.

---

## Part 1: General Approach for HPPS Assignments

### Step 1: Understand the Assignment Structure

HPPS assignments typically provide:

1. **Code skeleton** - Partially implemented files with `assert(0)` or empty function bodies
2. **Header files** - Define exact function signatures you must implement
3. **Makefile** - Build system already configured
4. **Driver programs** - Test harnesses that call your implementations
5. **Data format specifications** - Binary or text file formats for I/O

**What to do:**
- Read ALL provided header files first to understand the API contract
- Identify which functions contain `assert(0)` - these need implementation
- Run `make` to see what compiles and what doesn't
- Read the driver programs to understand how your code will be called

### Step 2: Identify the Implementation Tasks

Break down the assignment into distinct modules:

| Module Type | Typical Purpose | Example from A3 |
|-------------|-----------------|-----------------|
| I/O | Read/write data files | `io.c` |
| Utilities | Helper functions | `util.c` |
| Core Algorithm | Main logic | `kdtree.c`, `bruteforce.c` |
| Data Structures | Custom types | `struct node`, `struct kdtree` |

### Step 3: Follow the Dependency Order

Implement in order of dependencies:

```
1. I/O functions (no dependencies)
2. Utility functions (may depend on I/O)
3. Simple algorithm (depends on utilities)
4. Complex algorithm (depends on utilities, may reuse simple algorithm for testing)
```

### Step 4: Match the Exact Specifications

HPPS assignments are strict about:

- **Function signatures** - Must match header exactly
- **Return values** - NULL on error, 0/1 for success/failure as specified
- **Memory ownership** - Who allocates, who frees
- **Data formats** - Exact byte order, sizes, types

---

## Part 2: Common Implementation Patterns

### Binary File I/O

HPPS frequently uses binary file formats for efficiency.

**Pattern:**
```c
Type* read_data(FILE *f, int *size_out) {
    // 1. Read header (sizes, dimensions, etc.)
    int32_t n;
    if (fread(&n, sizeof(int32_t), 1, f) != 1) {
        return NULL;  // Error: couldn't read header
    }

    // 2. Validate before allocation
    if (n <= 0) {
        return NULL;  // Error: invalid size
    }

    // 3. Allocate with overflow protection
    Type *data = malloc((size_t)n * sizeof(Type));
    if (!data) {
        return NULL;  // Error: allocation failed
    }

    // 4. Read data, clean up on failure
    if (fread(data, sizeof(Type), n, f) != (size_t)n) {
        free(data);
        return NULL;  // Error: short read
    }

    // 5. Return results
    *size_out = n;
    return data;
}
```

**Key points:**
- Use `int32_t` for portable binary formats
- Always check `fread`/`fwrite` return values
- Cast to `size_t` for large allocations
- Free memory on partial failure
- Return NULL on any error

### Memory Management

**Pattern: Caller-frees**
```c
// Function allocates, caller must free
int* compute_result(int n) {
    int *result = malloc(n * sizeof(int));
    // ... fill result ...
    return result;  // Caller responsible for free()
}
```

**Pattern: Struct with cleanup function**
```c
struct thing *thing_create(...) {
    struct thing *t = malloc(sizeof(struct thing));
    // ... initialize ...
    return t;
}

void thing_free(struct thing *t) {
    // Free internal allocations first
    free(t->internal_data);
    // Then free the struct itself
    free(t);
}
```

### Recursive Data Structures

For trees, linked lists, etc.:

**Construction pattern:**
```c
struct node* create_node(int depth, int n, int *data) {
    // Base case
    if (n == 0) return NULL;

    // Create this node
    struct node *node = malloc(sizeof(struct node));
    node->value = data[n/2];  // or whatever selection logic

    // Recurse
    node->left = create_node(depth + 1, n/2, data);
    node->right = create_node(depth + 1, n - n/2 - 1, data + n/2 + 1);

    return node;
}
```

**Destruction pattern:**
```c
void free_node(struct node *node) {
    if (node == NULL) return;

    // Free children first (post-order)
    free_node(node->left);
    free_node(node->right);

    // Then free this node
    free(node);
}
```

### Algorithm Implementation

**Simple/Brute-force first:**
```c
// Always implement the simple O(n) or O(n^2) version first
// Use it to validate the optimized version
int* simple_algorithm(int n, const double *data) {
    int *result = malloc(...);
    for (int i = 0; i < n; i++) {
        // Simple, obviously correct logic
    }
    return result;
}
```

**Then optimize:**
```c
// Optimized version should produce identical results
int* optimized_algorithm(int n, const double *data) {
    // Build data structure
    struct tree *t = build_tree(n, data);

    // Use data structure for faster queries
    int *result = query_tree(t, ...);

    // Clean up
    free_tree(t);
    return result;
}
```

---

## Part 3: Testing Strategy

### 1. Compile-Test Loop
```bash
# After each function implementation:
make clean && make
./test-program  # Run provided tests
```

### 2. Cross-Validation
```bash
# Compare simple vs optimized implementations
./simple-version input.dat > output1.dat
./optimized-version input.dat > output2.dat
cmp output1.dat output2.dat  # Should be identical
```

### 3. Edge Cases
Test with:
- Empty input (n = 0)
- Single element (n = 1)
- Small inputs (n = 2, 3, 4) for manual verification
- Large inputs for performance

### 4. Memory Checking
```bash
valgrind --leak-check=full ./program input.dat
```

---

## Part 4: Common Mistakes to Avoid

### 1. Integer Overflow
```c
// BAD: overflow for large n
int *data = malloc(n * sizeof(int));

// GOOD: cast to size_t
int *data = malloc((size_t)n * sizeof(int));
```

### 2. Forgetting Error Checks
```c
// BAD: no error handling
data = malloc(n * sizeof(int));
fread(data, sizeof(int), n, f);

// GOOD: check everything
data = malloc(n * sizeof(int));
if (!data) return NULL;
if (fread(data, sizeof(int), n, f) != n) {
    free(data);
    return NULL;
}
```

### 3. Wrong Types in Binary I/O
```c
// BAD: 'int' size varies by platform
int n;
fread(&n, sizeof(int), 1, f);

// GOOD: fixed-size types
int32_t n;
fread(&n, sizeof(int32_t), 1, f);
```

### 4. Memory Leaks in Error Paths
```c
// BAD: leaks 'a' if 'b' allocation fails
int *a = malloc(n * sizeof(int));
int *b = malloc(n * sizeof(int));
if (!b) return NULL;  // 'a' is leaked!

// GOOD: clean up on failure
int *a = malloc(n * sizeof(int));
if (!a) return NULL;
int *b = malloc(n * sizeof(int));
if (!b) {
    free(a);
    return NULL;
}
```

### 5. Off-by-One in Recursion
```c
// Common in tree construction: getting the split wrong
// If array has indices [0..n-1] and median is at m:
// - Left subtree: indices [0..m-1], count = m
// - Right subtree: indices [m+1..n-1], count = n - m - 1
// - NOT count = n - m (off by one)
```

---

## Part 5: Example Application (Assignment 3)

### What Was Required

| Task | Approach |
|------|----------|
| Binary I/O | `fread`/`fwrite` with `int32_t` headers |
| Distance function | Loop over dimensions, sum squares, sqrt |
| k-closest maintenance | Array of k slots, insertion sort |
| Brute-force k-NN | Loop all points, call insert_if_closer |
| k-d tree construction | Recursive median split |
| k-d tree search | Recursive with pruning conditions |

### Implementation Order

1. `io.c` - No dependencies, enables testing other code
2. `util.c: distance()` - Simple math, needed by everything
3. `util.c: insert_if_closer()` - Core logic for k-NN
4. `bruteforce.c` - Uses utilities, validates correctness
5. `kdtree.c: create` - Build the tree structure
6. `kdtree.c: knn` - Search with pruning

### Validation

```bash
# Generate test data
./knn-genpoints 1000 2 points.dat
./knn-genpoints 100 2 queries.dat

# Run both implementations
./knn-bruteforce points.dat queries.dat 5 brute-out.dat
./knn-kdtree points.dat queries.dat 5 tree-out.dat

# Compare results
cmp brute-out.dat tree-out.dat && echo "PASS"
```

---

## Summary Checklist

Before submitting any HPPS assignment:

- [ ] All functions matching header signatures
- [ ] All `assert(0)` replaced with actual implementations
- [ ] Error handling on all allocations and I/O
- [ ] Memory freed appropriately (no leaks)
- [ ] Correct types for binary I/O (`int32_t`, not `int`)
- [ ] Simple version works correctly
- [ ] Optimized version matches simple version output
- [ ] Edge cases tested (n=0, n=1, large n)
- [ ] Code compiles without warnings (`-Wall -Wextra`)
- [ ] Valgrind reports no memory errors


# very important

it is very important to note that when answering a assingmnet one should answer as if they were the best student in the world. also only editing code or adding files that they have been given explicite permission to answer through the pdf file.
