# **Overview**

To modify the cellular stuff, modify **gpu-code/particle.cl**. This is the file	
that contains the code that runs on the CPU/GPU in parallel.

***

# **Bugs and Fixes**

| Date | Description | Reason & Fix |
|:-|:-|:-|
| September 27-28, 2025 | The particles were rendering in hyper colorful blobs. | This was caused by the processing kernel thinking the species vector was a *float* instead of an *int*. This was because I forgot the change the `cl_mem` buffer DT from `sizeof(float)*N` to `sizeof(int)*N`. I needed to change this becasue the species vector was originally a float. not an *int*. |
| September 29, 2025 | Windows couldn't build because of BASH-style Makefile. | This was becasue I made the Makefile for Debian-based systems, not OS undependable. Fixed by adding `CMakeLists.txt`. |

***

# ***Screenshots***

<figure>
	<img width="1366" height="768" alt="Screenshot 2025-09-28 5 45 18 PM" src="https://github.com/user-attachments/assets/592072fd-ddc2-4468-a06d-3806098d7828" />
	<figcaption>Sep 28, 2025 @ 5:28 PM - The tuned version of the simulation so far.</figcaption>
</figure>

***

<figure>
	<img width="130" height="auto" alt="A 'rotifer' organism" src="https://github.com/user-attachments/assets/8823513e-226c-45de-ac0c-996e099e5bd4" />
	<img width="150" height="auto" alt="A 'rotifer' organism" src="https://github.com/user-attachments/assets/83e3f0d3-6e9d-4a68-9a7e-428618709a8c" />
	<img width="80" height="auto" alt="A 'rotifer' organism" src="https://github.com/user-attachments/assets/3a12dd4e-4c30-4f4d-a911-8845676cc582" />
	<img width="240" height="auto" alt="A 'rotifer' organism" src="https://github.com/user-attachments/assets/d1e96904-9dcc-41af-86b2-4d8f7cba6768" />
	<img width="205" height="auto" alt="A 'rotifer' organism" src="https://github.com/user-attachments/assets/49e31ad1-62e9-40ed-ad7a-ba5adeb7bf93" />
	<img width="150" height="auto" alt="A 'rotifer' organism" src="https://github.com/user-attachments/assets/cdb01788-ec61-499c-b4d8-f5701637580b" />
	<img width="170" height="auto" alt="image" src="https://github.com/user-attachments/assets/a955775c-aa62-454f-bc6b-f04f7b61795b" />
	<img width="115" height="auto" alt="image" src="https://github.com/user-attachments/assets/06fc6b62-5353-44ca-a0d9-4c13a98bc1a8" />
	<figcaption><br>Sep 28, 2025 @ 6-7 PM - Some 'rotifer' organism patterns.</figcaption>
</figure>

***

<figure>
	<img width="123" height="auto" alt="image" src="https://github.com/user-attachments/assets/28c4a4a5-5124-47e1-a922-be33e1789ff8" />
	<img width="183" height="auto" alt="image" src="https://github.com/user-attachments/assets/3e336799-d14f-44d2-9f29-396e7554356d" />
	<figcaption><br>Sep 28, 2025 @ 7:13 PM - Some 'helizoan' organism patterns.</figcaption>
</figure>

***

<figure>
	<img width="100" height="auto" alt="A 'jellyfish' organism" src="https://github.com/user-attachments/assets/cae97cd5-7d39-4781-b142-3ff1d8199a77" />
	<img width="100" height="auto" alt="A 'jellyfish' organism" src="https://github.com/user-attachments/assets/a43b42e2-cea9-4689-968b-4b6fb7a29346" />
	<img width="90" height="auto" alt="A 'jellyfish' organism" src="https://github.com/user-attachments/assets/52e635a7-7c0e-4da6-afba-953bb0fe5f1b" />
	<img width="120" height="auto" alt="A 'jellyfish' organism" src="https://github.com/user-attachments/assets/fffa4894-d55d-498f-9a51-15be611ea0c8" />
	<img width="100" height="auto" alt="A 'jellyfish' organism" src="https://github.com/user-attachments/assets/d782cd42-8b50-460a-a4da-ec49cf4914c1" />
	<figcaption>Sep 30, 2025 @ 1:24 PM - Some 'jellyfish' organisms.</figcaption>
</figure>
