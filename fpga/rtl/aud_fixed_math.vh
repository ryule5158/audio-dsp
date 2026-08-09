// Signed Q1.23 helpers. Include inside a module body.
function signed [23:0] aud_sat24;
    input signed [55:0] value;
    begin
        if (value > 56'sd8388607)
            aud_sat24 = 24'sh7fffff;
        else if (value < -56'sd8388608)
            aud_sat24 = -24'sd8388608;
        else
            aud_sat24 = value[23:0];
    end
endfunction

function signed [23:0] aud_mul_q23;
    input signed [23:0] a;
    input signed [23:0] b;
    reg signed [47:0] product;
    reg signed [55:0] scaled;
    begin
        product = a * b;
        scaled = product >>> 23;
        aud_mul_q23 = aud_sat24(scaled);
    end
endfunction

function signed [23:0] aud_add_sat;
    input signed [23:0] a;
    input signed [23:0] b;
    reg signed [55:0] sum;
    begin
        sum = a;
        sum = sum + b;
        aud_add_sat = aud_sat24(sum);
    end
endfunction

function signed [23:0] aud_mix_q23;
    input signed [23:0] dry;
    input signed [23:0] wet;
    input signed [23:0] mix;
    reg signed [23:0] dry_weight;
    reg signed [47:0] dry_product;
    reg signed [47:0] wet_product;
    reg signed [55:0] sum;
    begin
        if (mix <= 0)
            aud_mix_q23 = dry;
        else if (mix >= 24'sh7fffff)
            aud_mix_q23 = wet;
        else begin
            dry_weight = 24'sh7fffff - mix;
            dry_product = dry * dry_weight;
            wet_product = wet * mix;
            sum = (dry_product + wet_product) >>> 23;
            aud_mix_q23 = aud_sat24(sum);
        end
    end
endfunction
