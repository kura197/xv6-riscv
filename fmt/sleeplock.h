4150 // Long-term locks for processes
4151 struct sleeplock {
4152   uint locked;       // Is the lock held?
4153   struct spinlock lk; // spinlock protecting this sleep lock
4154 
4155   // For debugging:
4156   char *name;        // Name of lock.
4157   int pid;           // Process holding lock
4158 };
4159 
4160 
4161 
4162 
4163 
4164 
4165 
4166 
4167 
4168 
4169 
4170 
4171 
4172 
4173 
4174 
4175 
4176 
4177 
4178 
4179 
4180 
4181 
4182 
4183 
4184 
4185 
4186 
4187 
4188 
4189 
4190 
4191 
4192 
4193 
4194 
4195 
4196 
4197 
4198 
4199 
