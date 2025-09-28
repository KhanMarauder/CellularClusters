# **Overview**

To modify the cellular stuff, modify **gpu-code/particle.cl**. This is the file  
that contains the code that runs on the CPU/GPU in parallel.

***

# **Bugs and Fixes**

| Date               | Description                                           | Reason & Fix                                                                                                    |
|:-------------------|:------------------------------------------------------|:----------------------------------------------------------------------------------------------------------------|
| September 27-28, 2025 | The particles were rendering in hyper colorful blobs. | This was caused by the processing kernel thinking the species vector was a *float* instead of an *int*. This was because I forgot the change the `cl_mem` buffer DT from `sizeof(float)*N` to `sizeof(int)*N`. I needed to change this becasue the species vector was originally a float. not an *int*.|

***

# ***Screenshots***
<img width="1366" height="768" alt="Screenshot 2025-09-28 5 45 18 PM" src="https://github.com/user-attachments/assets/592072fd-ddc2-4468-a06d-3806098d7828" />
