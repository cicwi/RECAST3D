import numpy as np

space = np.load(
    '/export/scratch2/buurlage/rec_sirt_animated_gears_broken_I5000.npy')
space[space < 0] = 0.0
clamp = np.max(space) / 5
space[space > clamp] = clamp
space = np.flip(np.transpose(space, axes=[1, 0, 2]), 1)
space = space[::2, ::2, ::2]
space = np.pad(space, ((300, 300), (125, 125), (0, 0)), 'median',
               stat_length=20)  # 'constant', constant_values=0)
np.save('/export/scratch2/buurlage/rec_sirt_animated_gears_broken_I5000_cleaned.npy',
        space)

projections = np.load(
    '/export/scratch2/buurlage/radio_animated_gears_broken_I5000.npy')
projections = np.pad(projections, ((232, 232), (0, 0), (28, 28)), 'median',
                     stat_length=20)  # 'constant', constant_values=0)
projections[projections < 0] = 0
projections = projections[::4, :, ::4]
print (projections.shape)

np.save('/export/scratch2/buurlage/radio_animated_gears_broken_I5000_cleaned.npy',
        projections)
