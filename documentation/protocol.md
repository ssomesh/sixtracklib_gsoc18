# Protocol definition



Object is allocated in a flat buffer



```c
struct Buffer {
    uintptr_t self;
    size_t size;
    size_t nobjects;
    size_t npointers;
    size_t typedefs_size;
    void * data;
    uintptr_t * objects;
    uintptr_t * types;
    uintptr_t * pointers;
    char * typedefs;
}
```

Memory structure

| Size          | Name                    | Description                          |
| ------------- | ----------------------- | ------------------------------------ |
| 8             | self                    | Address in memory of the buffer      |
| 8             | size                    | size of the buffer                   |
| 8             | data                    | Pointer to the beginning of data     |
| 8             | nobjects                | Number of objects stored             |
| 8             | objects                 | Pointer to the list of objects       |
| 8             | npointers               | Number of pointers in the data       |
| 8             | pointers                | Pointer to the list of objects       |
| size          | data[size]              | Serialized data                      |
| 8*nobjects    | object[nobjects]        | Serialized objects                   |
| 8*npointers   | pointer[npointers]      | Pointers to data containing pointers |
| 8*nobjects    | types[nobjects]         | Type id of each pointer              |
| typedefs_size | typedefs[typedefs_size] | Description of the types             |

