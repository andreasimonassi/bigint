'use strict';
var mp =
    (function () {

        function GetAsManyDigitsFromA(A, B) {
            let Ai = A.substring(0, B.length);
            if (Ai < B)
                Ai = A.substring(0, B.length + 1);
            return Ai;
        }
        function digitFromRightHex(Num, ix) {
            if (Num.length <= ix)
                return 0;
            let c = Num.charAt(Num.length - 1 - ix).toUpperCase().charCodeAt(0) - 48;

            if (c >= 0 && c <= 9)
                return c;
            c -= 7;
            if (c >= 10 && c <= 15)
                return c;
            throw "Input was not a valid hex string";
        }

        function digitFromLeftHex(Num, ix) {
            if (Num.length <= ix)
                throw "index out of range in digitFromLeft(Num, ix)";

            let c = Num.charAt(ix).toUpperCase().charCodeAt(0) - 48;
            if (c >= 0 && c <= 9)
                return c;
            c -= 7;
            if (c >= 10 && c <= 15)
                return c;
            throw "Input was not a valid hex string";
        }


        function digitFromRightDecimal(Num, ix) {
            if (Num.length <= ix)
                return 0;
            let c = Num.charCodeAt(Num.length - 1 - ix) - 48;
            if (c < 0 || c > 9)
                throw "Input was not a valid decimal string";
            return c;
        }

        function digitFromLeftDecimal(Num, ix) {
            if (Num.length <= ix)
                throw "index out of range in digitFromLeft(Num, ix)";

            let c = Num.charCodeAt(ix) - 48;
            if (c < 0 || c > 9)
                throw "Input was not a valid decimal string";
            return c;
        }

        var base = 10;
        var digitFromLeft = digitFromLeftDecimal;
        var digitFromRight = digitFromRightDecimal;

        function getRightDigits(Num, num) {
            //given a number like "123456" , if we want to keep its 3 right digits:  we return "456"
            //but also    
            return Num.substring(Num.length - num);

        }

        function SingleDigitByLongMultiply(A, B) {
            var result = "";
            var b = digitFromLeft(B, 0);

            if (A.length < 1)
                throw "A input len no good";

            for (let i = 0; i < A.length; ++i) {
                let a = digitFromRight(A, i);

                if (a < 0 || a >= base)
                    throw "A Input was not a valid decimal string";

                let m = (a * b).toString(base);

                let r_low = digitFromRight(result, i);

                let low = digitFromRight(m, 0);   // multiplication low digit result
                let high = digitFromRight(m, 1); //multiplication high digit result

                r_low = (low + r_low); //sum old result + low digit result

                if (r_low >= base) //carry detection
                {
                    high++; //have carry
                    r_low %= base;
                }

                let trunc = getRightDigits(result, i);

                result = (high * base + r_low).toString(base) + trunc;
            }
            return trimZeroes(result);
        }

        function LongMultiplyWithIntermediateResult(A, B) {
            var intermediate = [];
            let len = B.length;
            var sum;
            var result = "0";

            for (let i = 0; i < len; ++i) {
                let temp = SingleDigitByLongMultiply(A, digitFromRight(B, i).toString(base));
                intermediate.push(temp);
                var padding = Array(1 + i).join('0');
                result = LongSum(result, temp + padding);
            }

            return {
                intermediate: intermediate,
                result: result
            };

        }

        function LongCompare(A, B) {
            let len = A.length > B.length ? A.length : B.length;
            for (let i = 0; i < len; ++i) {
                let a = digitFromRight(A, len - 1 - i);
                let b = digitFromRight(B, len - 1 - i);
                if (a < b)
                    return -1;
                if (a > b)
                    return 1;
            }
            return 0;
        }

        function LongSubtraction(A, B) {
            if (LongCompare(A, B) < 0)
                throw "That operation is not defined in N";
            let result = "";
            let len = A.length > B.length ? A.length : B.length;
            let borrow = 0;

            for (let i = 0; i < len; ++i) {
                let a = digitFromRight(A, i);
                let b = digitFromRight(B, i);
                let c = 0;
                if (a - borrow < b) {
                    c = base + a - b - borrow;
                    borrow = 1;
                }
                else {
                    c = a - b - borrow;
                    borrow = 0;
                }

                result = c.toString(base) + result;
            }

            return trimZeroes(result);
        }

        function LongSum(A, B) {
            let result = "";
            let len = A.length > B.length ? A.length : B.length;
            let carry = 0;

            for (let i = 0; i < len; ++i) {
                let a = digitFromRight(A, i);
                let b = digitFromRight(B, i);
                let c = a + b + carry;
                if (c >= base) {
                    carry = 1;
                    c = c % base;
                }
                else
                    carry = 0;

                result = c.toString(base) + result;
            }

            if (carry)
                result = "1" + result;

            return result;
        }

        function trimZeroes(A) {
            let pos = 0;
            while (pos < A.length && (A.charAt(pos) === '0' || A.charAt(pos) ===' ' ) )
                pos++;

            let r = A.substring(pos);
            if (r === '')
                r = '0';

            return r;
        }

        function GuessDivisor(Ai, B) {
            let Bm = digitFromLeft(B, 0);
            let Aguess = digitFromLeft(Ai, 0);

            if (Aguess === Bm) {
                if (Ai.length === B.length)
                    return 1;
                return base-1;
            }
            else if (Aguess > Bm)
                return (~~(Aguess / Bm));
            else {
                if (Ai.length === 1)
                    return 0;
                Aguess = Aguess * base + digitFromLeft(Ai, 1);
                return (~~(Aguess / Bm));
            }

        }

        function LongDivision(A, B) {
            A = trimZeroes(A.toLower());
            B = trimZeroes(B.toLower());

            let steps = [];

            if (B.length === 0 || LongCompare(B, "0") === 0)
                throw "Division by zero";
            if (A.length === 0 || A === "0") {
                return { Q: "0", R: "0", Steps: steps };
            }

            let cmp = LongCompare(A, B);
            if (cmp === 0)
                return { Q: "1", R: "0", Steps: steps };
            if (cmp < 0)
                return { Q: "0", R: A, Steps: steps };
            let Q = "";
            let R = "0";

            let Ai = GetAsManyDigitsFromA(A, B);
            A = A.substring(Ai.length);

            let StateLooping = true;
            while (StateLooping) {
                Ai = trimZeroes(Ai);
                //start finding the greatest Qi that satisfies Qi*B <=Ai 
                let Qi = GuessDivisor(Ai, B);
                let temp = SingleDigitByLongMultiply(B, Qi.toString(base));
                while (LongCompare(temp, Ai) > 0) {
                    Qi--;
                    temp = SingleDigitByLongMultiply(B, Qi.toString(base));
                }
                //end finding the greatest Qi that satisfies Qi*B <=Ai 
                var padding = A.length + 1;
                let step = { Ai: Ai, Minuend: temp, Diff: "", Qi: Qi.toString(base), Padding: padding };

                Q += Qi.toString(base); //Append qi to Q
                //temp = long_multiply already computed...
                Ai = LongSubtraction(Ai, temp);
                step.Diff = Ai;

                steps.push(step);
                if (A.length > 0) {
                    Ai += A.charAt(0);
                    A = A.substring(1);
                }
                else
                    StateLooping = false;
            }
            return { Q: Q, R: Ai, Steps: steps };

        }

        return {
            LongDivision: LongDivision,
            LongSum: LongSum,
            LongSubtraction: LongSubtraction,
            LongMultiply: LongMultiplyWithIntermediateResult,
            SetBase10: () => {
                digitFromLeft = digitFromLeftDecimal;
                digitFromRight = digitFromRightDecimal;
                base = 10;
            },
            SetBase16: () => {
                base = 16;
                digitFromLeft = digitFromLeftHex;
                digitFromRight = digitFromRightHex;
            },
            LongCompare: LongCompare,
            TrimZeroes: trimZeroes
        };

})();