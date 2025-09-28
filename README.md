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
	<img width="260" height="200" alt="A 'rotifer' organism" src="https://github.com/user-attachments/assets/8823513e-226c-45de-ac0c-996e099e5bd4" />
	<img width="315" height="195" alt="A 'rotifer' organism" src="https://github.com/user-attachments/assets/83e3f0d3-6e9d-4a68-9a7e-428618709a8c" />
	<img width="170" height="265" alt="A 'rotifer' organism" src="https://github.com/user-attachments/assets/3a12dd4e-4c30-4f4d-a911-8845676cc582" />
	<img width="480" height="245" alt="A 'rotifer' organism" src="https://github.com/user-attachments/assets/d1e96904-9dcc-41af-86b2-4d8f7cba6768" />
	<figcaption><br>Sep 28, 2025 @ 6:49 PM - Some 'rotifer' organisms.</figcaption>
</figure>

***
