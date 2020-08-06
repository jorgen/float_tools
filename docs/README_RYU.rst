Converting Float to String and vise versa

Lets look at converting some different half float values to a integer
significand and exponent. Its easy to print a integer significand and exponent
to string using division and modulo.  

Single float is 1 bit sign, 8 bit exponent and 23 bit fraction. It has 127 as a bias.


Lets start with converting this float formats maximum value `6442450944.0` or binary representation `10011111 10000000000000000000000`
First we extract the exponent and mentissa.
```
e = 159, m = 4194304 
```

Then we apply bias to the exponent.
```
e = 32, m = 4194304 
```

The value of the number can be calculated by doing:
```
pow(2, 32) * (1 + (4194304 / 8388608))
or
pow(2, 32) * (1 + (4194304 * pow(2,-23)))
```

We move the comma out of the mentissa expression by adding `pow(2,
mentissa_width)`, but then we have to subtract `mentissa_width` from the
exponent. 
```
e = 9, mentissa = 12582912
pow(2,9) * 12582912 = 6442450944
```

Later we will need some extra precision to seperate the half values between the
next float value up and down. We therefor shift the mentissa up 2 places. We
then also have to subtract 2 from the exponent.
```
e = 7, mentissa = 50331648
pow(2,7) * 50331648 = 6442450944
```

Lets call the exponent and mentissa at this stage for `e2` and `m2`.
```
e2 = 7
m2 = 50331648
```

Now lets calculate some variables that will help us calculating a base 10 significand and the exponent.

```
q = max(floor(log_10(pow(2,e2))) - 1, 0)
q = max(floor(e2 * log_10(2)) - 1, 0)
q = max(floor(7 * log_10(2)) - 1, 0)
q = max(1, 0)
q = 1

k = scale + floor(log_2(pow(5,q)))
k = scale + floor(q * log_2(5))
k = scale + floor(1 * log_2(5))
k = 2 (for scale = 0)

cached value c = pow(2, k) / pow(5, q)
             c = pow(2, 2) / pow(5, 1)
             c = 0.8 
```

The scale is there so that we ensure the cache value does not have a fraction. For now we will just leave it as 0.

We can now calculate the Significand and the Base 10 Exponent.
```
Significand = (m2 * c) / pow(2, -e2 + q + k)
Significand = (50331648 * 0.8) / pow(2, -7 + 1 + 2)
Significand = 644245094.4000001
Exponent    = q
Exponent    = 1 
```

Lets try and convert the digit `3.0` to a string or in binary form `1000 1000`.

First we extract the exponent and mentissa.
```
e = 8, m = 8 
```

Then we apply bias to the exponent.
```
e = 1, m = 8 
```

The value of the number can be calculated by doing:
```
2^1 * (1 + (8 / 16))
or
2^1 * (1 + (8 * 2^-4))
```

We move the comma out of the mentissa expression by adding `1 <<
mentissa_width`, but then we have to subtract `mentissa_width` from the
exponent. 
```
e = -3, mentissa = 24 
2^-3 * 24 = 3.0
```

Later we will need some extra precision to seperate the half values between the
next float value up and down. We therefor shift the mentissa up 2 places. We
then also have to subtract 2 from the exponent.
```
e = -5, mentissa = 96 
2^-5 * 96 = 3.0
```

Lets call the exponent and mentissa at this stage for `e2` and `m2`.
```
e2 = -5
m2 = 96 
```

Now lets calculate some variables that will help us calculating a base 10 significand and the exponent.
```
q = floor(-e2 * log_10(5)) - 1
q = floor(5 * log_10(5)) - 1
q = 2

k = ceil((-e2 - q) * log_2(5)) - integer_scale
k = ceil((5 - 2) * log_2(5)) - integer_scale
k = 7 - integer_scale

cached value c = 5^(-e2 - q) / 2^k
             c = 5^(5 - 2) / 2 ^ (7)
             c = 0.9765625 (for integer_scale = 0)
```

The integer_scale is there so that we ensure the cache value does not have a fraction. For now we will just leave it as 0.

We can now calculate the Significand and the Base 10 Exponent.
```
Significand = (m2 * c) / 2^(q - k)
Significand = (96 * 0.9765625) / 2^(2 - 7)
Significand = 3000.0
Exponent    = e2 + q
Exponent    = -5 + 2
Exponent    = -3
```

Converting string to ieee.

So first off is to parse the string. It is then parsed into a Significan and an Exponent.
