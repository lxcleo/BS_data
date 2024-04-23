clear all
close all
clc
%%
z1k = importdata('fake_sinft_30N.csv');
timestamp_z1k = z1k.data(:,2);
timestamp_z1k = diff(timestamp_z1k);
y_forcez_z1k = z1k.data(:,3);
figure(1)
% subplot(311)
plot(y_forcez_z1k)
title('force')

% Zpos = z1k.data(:,43);
% Zvel = diff(Zpos) ./ timestamp_z1k;
% Zacc = diff(Zvel) ./ timestamp_z1k(2:end);
% figure()
% plot(Zacc)
% title('Acceleration');

% hold on;
% 
% % plot(z1k.data(:,43))
% subplot(312)
% temp = diff(z1k.data(:,43));
% plot(z1k.data(:,43))
% title('Z_trajectory')
% 
% subplot(313)
% plot(temp)
% title('Z trajectory diff')
%%
% 
zfb = importdata('attach_zbnd_LR.csv');
timestamp_zfb = zfb.data(:,1);
timestamp_zfb = diff(timestamp_zfb);
figure(2)
subplot(311)
plot(zfb.data(:,3))
title('force')
% hold on;

% plot(z1k.data(:,43))
subplot(312)
temp = diff(zfb.data(:,43));
plot(zfb.data(:,43))
title('Z_trajectory')

subplot(313)
plot(temp)
title('Z trajectory diff')
%%
LR = importdata('attach_pureforce_LR.csv');
BS = importdata('attach_pureforce_BS.csv');
figure(3)
subplot(221)
axis equal
plot(LR.data(:,3))
title('Force Lower Right')
subplot(222)
axis equal
plot(diff(LR.data(:,3)))
title('Force Lower Right Diff')

subplot(223)
plot(BS.data(:,3))
title('Force Body Support')
subplot(224)
plot(diff(BS.data(:,3)))
ylim([-4 6])
title('Force Body Support Diff')

%%
% data_ns = importdata('drift_nosin.csv');
% data_s = importdata('drift_sin.csv');
data_ns = importdata('LR_drift_home.csv');
data_s = importdata('UL_drift_home.csv');

LR_ns = data_ns.data(end - 1,8:13)-data_ns.data(1,8:13);
UL_ns = data_s.data(end,8:13)-data_s.data(1,8:13);
lrns = rad2deg(LR_ns);
ulns = rad2deg(UL_ns);
norm(lrns);
norm(ulns);
% figure()
% plot(J_s_diff)