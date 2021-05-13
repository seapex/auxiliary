/*!
    Input:  val -- Initial voltage peak value.
*/
void Initialize(float val)
{
    IniFilterPar(val);
    int n = 400/avg_num_flicker() * 600;
    IniFlickerStatis(n);
}

    int i;
    for (i=0; i<160; i++) {
        if (i%10==0) pst_x_[0][phs] = 0;
        for (int j=0; j<rounds; j++) {
            for (int k=0; k<nums; k++) {
                data[k] = wave[k];
                data[k] /= 100;
            }
            int n = FlickerFilter(buf, data, nums, 0, phs);
            SetAvrgIns(buf, n, phs);
        }
        pst_[i%10] = GetPst(phs, 0);
        printf("%6.3f \n", pst_[i%10]);
    }
