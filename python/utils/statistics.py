def sig_cov(m, m_bkg, cov_bkg, p):
    """
    Estimate the covariance of the signal from the sample by exploiting the knowledge
    of the means and the covariance of the background sample as well as the fraction
    of signal in the sample.

    The necessary inputs:
    - m ... NxM matrix of observations where N is the number of variables and M
      is the number of observations.
    - m_bkg a vector with N entries, where the i-th element corresponds to the mean
      of the i-th variable of the background sample.
    - cov_bkg ... the NxN covariance matrix of the background sample.
    - p ... the fraction of signal in the mixed sample.

    The covariance matrix of the signal sample is calculated as

    C_S = 1/p * (C - (1-p)*C_B - (1-p)/p * mu*mu^T - (1-p)/p * mu_B*mu_B^T
                               - (p-1)/p * mu*mu_B^T -(p-1)/p * mu_B*mu^T),

    where:
    - C ... the covariance matrix of the mixed sample
    - C_B ... the covariance matrix of the background sample
    - mu ... the mean (vector) of the mixed sample
    - mu_B ... the mean (vector) of the background sample
    - p ... the fraction of signal in the mixed sample
    """
    import numpy as np
    # COULDDO: some basic error checking (matching of dimensions, etc...)

    shape = np.shape(m)
    cov = np.cov(m)

    # reshape the mean vectors into something that is understood as matrix
    # by numpy, so that the basic matrix operations work as expected
    mu = np.reshape(np.average(m, axis=1), (shape[0], 1))
    mu_B = np.reshape(m_bkg, (shape[0], 1))

    # precalculate some of the factors for slightly better readability below
    f_p = (p - 1) / p # == - (1 - p)/p
    mu_mu = mu.dot(mu.T)
    muB_muB = mu_B.dot(mu_B.T)
    mu_muB = mu.dot(mu_B.T)
    muB_mu = mu_B.dot(mu.T)

    return (cov - (1-p) * cov_bkg + f_p * (mu_mu + muB_muB) - f_p * (mu_muB + muB_mu)) / p


def corr_matrix(cov):
    """
    Calculate the correlation matrix from the passed covariance matrix as:

    corr = diag(cov)^{-1/2} * cov * diag(cov)^{-1/2}
    """
    import numpy as np

    # get the diagonal elements of the cov and raise them to the power of -1/2
    diag_vals = np.diag(cov)
    diag_vals = diag_vals**(-0.5)
    # form the diagonal values into a matrix again
    diag_cov = np.diag(diag_vals)

    return diag_cov.dot(cov).dot(diag_cov)


def sig_corr(m, m_bkg, cov_bkg, p):
    """
    Estimate the signal correlation matrix from the sample by exploiting the knowledge
    of the means and the covariance of the background sample as well as the fraction
    of signal in the sample.

    For more details on how the covariance of the signal is estimated see the docstring
    of sig_cov.
    """
    import pandas as pd
    cov_sig = sig_cov(m, m_bkg, cov_bkg, p)
    corr = corr_matrix(cov_sig)

    # if we have m as a pandas.DataFrame return also a data frame, where the columns
    # and indices are simply the indices of the original DataFrame
    # This allows to use seaborn.heatmap with labeling
    if isinstance(m, pd.DataFrame):
        return pd.DataFrame(corr, index=m.index, columns=m.index)

    return corr
