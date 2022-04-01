# parser-generator
## Parser generator grammar:

- ### Plain strings
```
rule_1 : 'hello'
rule_2 : ' world, \'standard escaping works\''
```
Strings **must** be non-empty. 

- ### Non-terminal rules

Name **must not** use upper-case letters.
```
first_rule : 'one ' second_rule
second_rule : 'two ' 'three'
```
same as
```
first_rule : 'one ' second_rule
second_rule : 'two three'
```
and
```
first_rule : 'one two three'
```
- ### Regular expression

Firstly, define regular expression token (name **must not** use lower-case letters)

```
NUMBER_TOKEN : '\\d+'
```
And then use in non-terminal rules

```    
number_rule : 'user ID = ' NUMBER_TOKEN
```
Order of definition and usage does not matter.

Names `EPS` and `END` are reserved and **must not** be declared, but `EPS` may be used in rules as empty string.
- ### Variables in rule

You may write `(var_name=smth)` to save the result of `smth` parsing. The `smth` can be:

1) #### Text. 
    
```
sign_rule : (sign='+')
sign_rule : (sign='-')
sign_rule : (sign='*')
sign_rule : (sign='/')
```

In this case, variables with type `std::string` and associated name will be generated. All such variables will contain the result of parsing.

2) #### Regular expression

```
ID : '[a-zA-Z]+'
mail : (username=ID) '@' (server=ID) '.' (domain=ID)
```

In this case, variables with type `std::string` and associated name will be generated. All such variables will contain the result of parsing.

3) #### Rule

```
two_mails : (mail1=mail) '\n' (mail2=mail)
```

In this case, the rule containing assignment will contain all exported variables of the assigned rule with an additional prefix same as the variable name.

- ### Functions

Parser generator supports user defined functions. 

```
$FUNCTION_NAME[list of <var type> <var_name> separated by comma] {
    multiline c++ code
}
```

Function name **must** start with dollar sign. The variable list indicates the variables that will be exported in the rule calling this function.

- ### Inheriting of variables

After the rule name you may write list of inherited variables for the rule separated by comma. These variables will be passed to the rule when it's parsing starts. 

Inherited variables for the rule is the union of inherited variables for each rule instance.

```
add : (l=num) (tail=add_tail) $SAVE_TAIL_VAL

add_tail[double l_val] : EPS $SAVE_L_VAL
add_tail : '+' (r=num) $ADD (tail=add_tail) $SAVE_TAIL_VAL
add_tail : '-' (r=num) $SUB (tail=add_tail) $SAVE_TAIL_VAL

num : (num=NUM) $SAVE_NUM

$ADD[double l_val] {
    l_val += r_val;
}

$SAVE_L_VAL[double val] {
    val = l_val;
}

$SAVE_TAIL_VAL[double val] {
    val = tail_val;
}

$SAVE_NUM[double val] {
    val = std::stod(num, nullptr);
}

NUM : '\\d+'
```

Code above evaluates expression in the form of `NUM ('+' NUM)*`.

- ### Starting rule

Starting rule **must** be non-terminal rule (i.e. it's name must not use upper-case letters) and be placed in the last line of the input file.

## Example

This grammar implements a calculator supporting adding, subtraction, multiplication, dividing and module operations

```
add_sub : (l=mul_div) (tail=add_sub_tail) $SAVE_TAIL_VAL

add_sub_tail[double l_val] : EPS $SAVE_L_VAL
add_sub_tail : '+' (r=mul_div) $ADD (tail=add_sub_tail) $SAVE_TAIL_VAL
add_sub_tail : '-' (r=mul_div) $SUB (tail=add_sub_tail) $SAVE_TAIL_VAL

mul_div : (l=num) (tail=mul_div_tail) $SAVE_TAIL_VAL
mul_div_tail[double l_val] : EPS $SAVE_L_VAL
mul_div_tail : '*' (r=num) $MUL (tail=mul_div_tail) $SAVE_TAIL_VAL
mul_div_tail : '/' (r=num) $DIV (tail=mul_div_tail) $SAVE_TAIL_VAL

num : '(' (inner=add_sub) ')' $SAVE_INNER
num : '|' (inner=add_sub) '|' $CALC_ABS
num : (num=NUM) $SAVE_NUM

$ADD[double l_val] {
    l_val += r_val;
}

$SUB[double l_val] {
    l_val -= r_val;
}

$MUL[double l_val] {
    l_val *= r_val;
}

$DIV[double l_val] {
    l_val /= r_val;
}

$SAVE_L_VAL[double val] {
    val = l_val;
}

$SAVE_TAIL_VAL[double val] {
    val = tail_val;
}

$SAVE_INNER[double val] {
    val = inner_val;
}

$SAVE_NUM[double val] {
    val = std::stod(num, nullptr);
}

$CALC_ABS[double inner_val] {
    val = std::abs(inner_val);
}

NUM : '\\d+'

add_sub
```

## Building and running

Use `build.sh` script and then use `run.sh` script to run the generator.

```
chmod +x build.sh run.sh
./build.sh
./run.sh <path to the grammar file> <path to the folder to place generated files>
```
