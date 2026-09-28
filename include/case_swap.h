#pragma once

/*
 * Swap the case of an ASCII letter: 'a'..'z' -> 'A'..'Z' and 'A'..'Z' ->
 * 'a'..'z'. Every other character (digits, punctuation, control codes,
 * bytes >= 0x80) is returned unchanged.
 */
char switch_case(char c);
