# **Overview**

To modify the cellular stuff, modify **gpu-code/particle.cl**. This is the file	
that contains the code that runs on the CPU/GPU in parallel.

***

# **Bugs and Fixes**

| Date | Description | Reason & Fix |
|:-|:-|:-|
| September 27-28, 2025 | The particles were rendering in hyper colorful blobs. | This was caused by the processing kernel thinking the species vector was a *float* instead of an *int*. This was because I forgot the change the `cl_mem` buffer DT from `sizeof(float)*N` to `sizeof(int)*N`. I needed to change this becasue the species vector was originally a float. not an *int*.|

***

# ***Screenshots***

<figure>
	<img width="1366" height="768" alt="Screenshot 2025-09-28 5 45 18 PM" src="https://github.com/user-attachments/assets/592072fd-ddc2-4468-a06d-3806098d7828" />
	<figcaption>Sep 28, 2025 @ 5:28 PM - The tuned version of the simulation so far.</figcaption>
</figure>

***

<figure>
	<img width="52" height="40" alt="image" src="https://github.com/user-attachments/assets/8823513e-226c-45de-ac0c-996e099e5bd4" />
	<figcaption>Sep 28, 2025 @ 6:49 PM - A 'Rotifer' organism.</figcaption>
</figure>

***
