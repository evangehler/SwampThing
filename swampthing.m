%% Load WaveForms CSV: columns [Time(s), C1(saw), C2(filtered PWM)]
T   = readtable('log.csv');   % <-- your file
t   = T{:,1};
saw = T{:,2};                        % C1 = saw

Fs = 1/median(diff(t));
x  = saw - mean(saw);

% --- Estimate fundamental freq from FFT (10 Hz .. 10 kHz window) ---
N   = numel(x);
X   = abs(fft(x .* hann(N)));
f   = (0:N-1)*(Fs/N);
mask = f>=10 & f<=1e4;                         % adjust if needed
[~,k] = max(X(mask));
f0 = f(mask);
f0 = f0(k);
if isempty(f0) || ~isfinite(f0), f0 = 500; end % fallback 500 Hz

% --- Peak-pick reset edges on negative derivative with safe MPD ---
dx  = [0; diff(saw)];
MPD = max( round(0.5*Fs/f0), 10 );             % ~half-period in samples
prom= 0.2*max(abs(dx));                         % derivative prominence
[~,loc] = findpeaks(-dx, 'MinPeakDistance', MPD, 'MinPeakProminence', prom);

if numel(loc) < 3
    error('Still too few cycles. Widen freq window or lower prominence.');
end

% --- Per-cycle slope from middle 60% of each ramp ---
M = numel(loc)-1;
slope = nan(M,1);
for i = 1:M
    i0 = loc(i); i1 = loc(i+1)-1;
    if i1 <= i0+10, continue; end
    m0 = i0 + round(0.20*(i1-i0));
    m1 = i0 + round(0.80*(i1-i0));
    p  = polyfit(t(m0:m1), saw(m0:m1), 1);
    slope(i) = p(1);                       % [V/s]
end
slope = slope(isfinite(slope));

% --- Histogram + Gaussian overlay ---
figure; histogram(slope,'Normalization','pdf'); hold on;
xlabel('Saw slope dV/dt [V/s]'); ylabel('PDF'); title('Per-cycle slope');
pd = fitdist(slope,'Normal');
xx = linspace(min(slope), max(slope), 400);
plot(xx, pdf(pd,xx), 'LineWidth', 1.5);
fprintf('mean = %.3g V/s,  std = %.3g V/s  (N=%d)\n', mean(slope), std(slope), numel(slope));

% Optional sanity: QQ-plot
figure; qqplot(slope); grid on; title('QQ-plot: saw slope');
